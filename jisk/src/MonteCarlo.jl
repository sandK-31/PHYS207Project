module MonteCarlo

using CUDA
using ..Grid
using ..Phase

export run_mc!, MCResult

"""
    MCResult

- `S_accum`: CuArray{Float32, 3}(N_r, N_phi, N_z), total deposited packet weight at
  scattering events per voxel. Convert to a physical source function with
  `source_function_from_accum`.
- `E_abs`: CuArray{Float32, 3} of deposited weight at absorption events per voxel.
- `n_escaped`: CuArray{Int64} (scalar-like, length 1): packets that left the grid without absorption.
- `n_absorbed`: CuArray{Int64}: packets terminated by absorption.
- `n_rr_killed`: CuArray{Int64}: packets killed by Russian roulette past max scatter.
"""
struct MCResult
    S_accum::CuArray{Float32,3}
    E_abs::CuArray{Float32,3}
    n_escaped::CuArray{Int64,1}
    n_absorbed::CuArray{Int64,1}
    n_rr_killed::CuArray{Int64,1}
    n_scatters::CuArray{Int64,1}
end

# --- Minimal on-device RNG: xorshift64* (one u64 of state per thread) ---

@inline function _xs_next(state::UInt64)
    state ⊻= state << 13
    state ⊻= state >> 7
    state ⊻= state << 17
    return state
end

@inline function _u64_to_f32(x::UInt64)
    # top 24 bits -> Float32 in [0, 1)
    return Float32(x >> 40) * Float32(1.0f0 / Float32(1 << 24))
end

# SplitMix64 to expand a seed+tid into decorrelated 64-bit states.
@inline function _splitmix64(z::UInt64)
    z += 0x9E3779B97F4A7C15
    z = (z ⊻ (z >> 30)) * 0xBF58476D1CE4E5B9
    z = (z ⊻ (z >> 27)) * 0x94D049BB133111EB
    return z ⊻ (z >> 31)
end

@inline function rand_f32!(state::Ref{UInt64})
    state[] = _xs_next(state[])
    return _u64_to_f32(state[])
end

# --- Main kernel: one thread per photon packet, grid-stride over N_p ---

function _mc_kernel!(
        S_accum, E_abs,
        ρ,                       # (N_r, N_phi, N_z) density
        log_r_edges,             # (N_r+1,)
        N_r::Int32, N_phi::Int32, N_z::Int32,
        r_min::Float32, r_max::Float32,
        z_min::Float32, z_max::Float32,
        dphi::Float32, dz::Float32,
        κ_abs::Float32, κ_sca::Float32,
        ρκ_max::Float32,
        phase_kind::Int32, g_asym::Float32,
        N_p::Int64, max_scatter::Int32,
        base_seed::UInt64,
        n_escaped, n_absorbed, n_rr_killed, n_scatter_counter)

    tid = (blockIdx().x - Int32(1)) * blockDim().x + threadIdx().x
    stride = blockDim().x * gridDim().x

    κ_ext = κ_abs + κ_sca

    i = Int64(tid)
    while i <= N_p
        # Per-thread RNG state decorrelated via splitmix
        rng_state = Ref(_splitmix64(base_seed ⊻ reinterpret(UInt64, i)))

        # --- emit photon isotropically from origin, weight = 1 ---
        ξ1 = rand_f32!(rng_state)
        ξ2 = rand_f32!(rng_state)
        ux, uy, uz = Phase.sample_isotropic(ξ1, ξ2)
        x = 0.0f0; y = 0.0f0; z = 0.0f0
        w = 1.0f0

        n_scatters = Int32(0)
        alive = true

        while alive
            # Woodcock step
            ξ = rand_f32!(rng_state)
            ξ = ξ < 1.0f-30 ? 1.0f-30 : ξ
            Δs = -log(ξ) / ρκ_max
            x += Δs * ux
            y += Δs * uy
            z += Δs * uz

            # Escape: outside cylinder or z-range
            r_cyl = sqrt(x*x + y*y)
            if r_cyl > r_max || z < z_min || z > z_max
                CUDA.@atomic n_escaped[1] += Int64(1)
                alive = false
                break
            end

            # r_cyl < r_min: photon passed through the inner hole; keep going
            if r_cyl < r_min
                # no matter here; treat as null collision
                continue
            end

            # Cell indices
            ir, iphi, iz = Grid.cyl_indices_from_cart(
                x, y, z, log_r_edges, N_r, dphi, N_phi, z_min, dz, N_z)
            if ir == Int32(0) || iz == Int32(0)
                continue
            end

            ρ_here = @inbounds ρ[ir, iphi, iz]
            P_real = (ρ_here * κ_ext) / ρκ_max
            ξ_r = rand_f32!(rng_state)
            if ξ_r > P_real
                continue     # null collision
            end

            # Real collision: absorb or scatter?
            ξ_a = rand_f32!(rng_state)
            if ξ_a < κ_abs / κ_ext
                CUDA.@atomic E_abs[ir, iphi, iz] += w
                CUDA.@atomic n_absorbed[1] += Int64(1)
                alive = false
                break
            end

            # Scatter: deposit weight then resample direction
            CUDA.@atomic S_accum[ir, iphi, iz] += w
            CUDA.@atomic n_scatter_counter[1] += Int64(1)
            n_scatters += Int32(1)

            # Russian roulette after max_scatter
            if n_scatters > max_scatter
                ξ_rr = rand_f32!(rng_state)
                if ξ_rr < 0.5f0
                    CUDA.@atomic n_rr_killed[1] += Int64(1)
                    alive = false
                    break
                else
                    w *= 2.0f0
                end
            end

            # New direction from phase function
            ξ_μ = rand_f32!(rng_state)
            μ = phase_kind == Int32(0) ? (2.0f0*ξ_μ - 1.0f0) : Phase.sample_hg(g_asym, ξ_μ)
            ξ_φ = rand_f32!(rng_state)
            φs = 6.2831853f0 * ξ_φ
            ux, uy, uz = Phase.rotate_to_frame(ux, uy, uz, μ, φs)
        end

        i += Int64(stride)
    end
    return
end

"""
    run_mc!(grid, ρ_d; κ_abs, κ_sca, g, phase=:hg,
            N_p=Int64(1_000_000), threads=256, blocks=nothing,
            max_scatter=100, seed=UInt64(0xC0FFEE)) -> MCResult

Launch the GPU Monte Carlo. All densities and opacities are in cgs.
`ρ_d` must be a CuArray{Float32,3} of shape (N_r, N_phi, N_z).
"""
function run_mc!(grid::CylGrid{Float32}, ρ_d::CuArray{Float32,3};
                 κ_abs::Real, κ_sca::Real, g::Real=0.0,
                 phase::Symbol=:hg,
                 N_p::Integer=Int64(1_000_000),
                 threads::Integer=256, blocks=nothing,
                 max_scatter::Integer=100,
                 seed::UInt64=UInt64(0xC0FFEE_BEEF_CAFE))

    CUDA.allowscalar(false)

    ρ_max  = maximum(ρ_d)           # ρ_d is a CuArray; max via reduction, no scalar index
    κ_abs_f = Float32(κ_abs)
    κ_sca_f = Float32(κ_sca)
    κ_ext_f = κ_abs_f + κ_sca_f
    ρκ_max  = Float32(ρ_max) * κ_ext_f

    S_accum   = CUDA.zeros(Float32, grid.N_r, grid.N_phi, grid.N_z)
    E_abs     = CUDA.zeros(Float32, grid.N_r, grid.N_phi, grid.N_z)
    n_escape  = CUDA.zeros(Int64, 1)
    n_absorb  = CUDA.zeros(Int64, 1)
    n_rr      = CUDA.zeros(Int64, 1)
    n_scat    = CUDA.zeros(Int64, 1)

    log_r_edges_d = CuArray(grid.log_r_edges)

    phase_kind = phase === :iso ? Int32(0) : Int32(1)

    # Launch configuration
    nb = if blocks === nothing
        max(1, min(Int(ceil(Int64(N_p) / threads)), 65535))
    else
        Int(blocks)
    end

    @cuda threads=threads blocks=nb _mc_kernel!(
        S_accum, E_abs,
        ρ_d,
        log_r_edges_d,
        grid.N_r, grid.N_phi, grid.N_z,
        Float32(grid.r_min), Float32(grid.r_max),
        Float32(grid.z_min), Float32(grid.z_max),
        Float32(grid.dphi),  Float32(grid.dz),
        κ_abs_f, κ_sca_f, ρκ_max,
        phase_kind, Float32(g),
        Int64(N_p), Int32(max_scatter),
        seed,
        n_escape, n_absorb, n_rr, n_scat)

    CUDA.synchronize()

    return MCResult(S_accum, E_abs, n_escape, n_absorb, n_rr, n_scat)
end

"""
    source_function_from_accum(S_accum, ρ, V_voxel, κ_sca, L_star, N_p) -> Array{Float32,3}

Convert the raw scattering-event weight accumulator to a scattering source
function S_sca(r,φ,z) in cgs units of erg s^-1 cm^-2 sr^-1:

    S_sca = (Σ deposits) · (L★ / N_p) / (4π · ρ · κ_sca · V_voxel)

Voxels with ρ = 0 return zero. Operates in Float64 on host to avoid Float32
overflow on large voxel volumes (~10^40 cm^3), returns Float32 for storage.

Inputs may be CuArray{Float32} or Array{Float64}; both are handled.
"""
function source_function_from_accum(S_accum, ρ, V_voxel,
                                    κ_sca::Real, L_star::Real, N_p::Integer)
    S_h = Array{Float64}(_cpu(S_accum))
    ρ_h = Array{Float64}(_cpu(ρ))
    V_h = Array{Float64}(_cpu(V_voxel))
    pkt_energy = Float64(L_star) / Float64(N_p)
    out = similar(S_h, Float32)
    @inbounds for i in eachindex(S_h)
        denom = 4π * ρ_h[i] * Float64(κ_sca) * V_h[i]
        out[i] = denom > 0.0 ? Float32(S_h[i] * pkt_energy / denom) : 0.0f0
    end
    return out
end

_cpu(a::CuArray) = Array(a)
_cpu(a::AbstractArray) = a

export source_function_from_accum

end # module
