module RayTrace

using CUDA
using ..Constants
using ..Grid

export render_image, ObserverParams

"""
    ObserverParams

- `inclination`: angle between disk symmetry axis (+z) and the line-of-sight, rad.
  0 = face-on, π/2 = edge-on.
- `azimuth`: rotation around disk z-axis, rad.
- `N_px`, `N_py`: image resolution.
- `fov_cm`: square field of view in cm (image plane width = height).
- `n_steps`: number of sampling points along each ray through the bounding volume.
"""
Base.@kwdef struct ObserverParams
    inclination::Float32 = Float32(π/3)
    azimuth::Float32     = 0.0f0
    N_px::Int32          = Int32(256)
    N_py::Int32          = Int32(256)
    fov_cm::Float32      = Float32(250 * Constants.AU)
    n_steps::Int32       = Int32(512)
end

# Ray intersection with finite cylinder {r_cyl ≤ r_max, z_min ≤ z ≤ z_max}.
# Returns (t_enter, t_exit); t_exit ≤ t_enter signals no intersection.
@inline function _ray_cyl_box(x::Float32, y::Float32, z::Float32,
                              dx::Float32, dy::Float32, dz::Float32,
                              r_max::Float32, z_min::Float32, z_max::Float32)
    # z-slab
    if abs(dz) < 1.0f-20
        if z < z_min || z > z_max
            return 1.0f0, -1.0f0
        end
        tz_lo = -1.0f30; tz_hi = 1.0f30
    else
        t1 = (z_min - z) / dz
        t2 = (z_max - z) / dz
        tz_lo = min(t1, t2)
        tz_hi = max(t1, t2)
    end
    # infinite cylinder r = r_max
    A = dx*dx + dy*dy
    B = 2.0f0 * (x*dx + y*dy)
    C = x*x + y*y - r_max*r_max
    if A < 1.0f-20
        # ray parallel to z: constant r_cyl
        if C > 0.0f0
            return 1.0f0, -1.0f0
        end
        tc_lo = -1.0f30; tc_hi = 1.0f30
    else
        disc = B*B - 4.0f0*A*C
        if disc < 0.0f0
            return 1.0f0, -1.0f0
        end
        s = sqrt(disc)
        tc_lo = (-B - s) / (2.0f0 * A)
        tc_hi = (-B + s) / (2.0f0 * A)
    end
    t_enter = max(tz_lo, tc_lo, 0.0f0)
    t_exit  = min(tz_hi, tc_hi)
    return t_enter, t_exit
end

function _rt_kernel!(img,
                     ρ, S_sca, B_nu_r,
                     log_r_edges,
                     N_r::Int32, N_phi::Int32, N_z::Int32,
                     r_min::Float32, r_max::Float32,
                     z_min::Float32, z_max::Float32,
                     dphi::Float32, dz::Float32,
                     κ_abs::Float32, κ_sca::Float32,
                     include_thermal::Int32, include_scatter::Int32,
                     dobs_x::Float32, dobs_y::Float32, dobs_z::Float32,
                     ex::Float32, ey::Float32, ez::Float32,
                     fx::Float32, fy::Float32, fz::Float32,
                     N_px::Int32, N_py::Int32, fov::Float32,
                     n_steps::Int32, L0::Float32)

    pid = (blockIdx().x - Int32(1)) * blockDim().x + threadIdx().x
    total = N_px * N_py
    if pid > total
        return
    end

    jx = (pid - Int32(1)) % N_px
    jy = (pid - Int32(1)) ÷ N_px

    u = (Float32(jx) + 0.5f0) / Float32(N_px) - 0.5f0
    v = (Float32(jy) + 0.5f0) / Float32(N_py) - 0.5f0
    px = u * fov
    py = v * fov

    x0 = L0 * dobs_x + px * ex + py * fx
    y0 = L0 * dobs_y + px * ey + py * fy
    z0 = L0 * dobs_z + px * ez + py * fz
    rdx = -dobs_x; rdy = -dobs_y; rdz = -dobs_z

    t_enter, t_exit = _ray_cyl_box(x0, y0, z0, rdx, rdy, rdz, r_max, z_min, z_max)
    if t_exit <= t_enter
        @inbounds img[jx + Int32(1), jy + Int32(1)] = 0.0f0
        return
    end

    ds = (t_exit - t_enter) / Float32(n_steps)
    I = 0.0f0
    τ = 0.0f0
    κ_ext = κ_abs + κ_sca

    t = t_enter + 0.5f0 * ds
    for _ in Int32(1):n_steps
        xs = x0 + t * rdx
        ys = y0 + t * rdy
        zs = z0 + t * rdz
        r_cyl = sqrt(xs*xs + ys*ys)

        if r_cyl < r_min || r_cyl > r_max || zs < z_min || zs > z_max
            t += ds
            continue
        end

        ir, iphi, izi = Grid.cyl_indices_from_cart(xs, ys, zs, log_r_edges, N_r,
                                                   dphi, N_phi, z_min, dz, N_z)
        if ir == Int32(0) || izi == Int32(0)
            t += ds
            continue
        end

        ρv = @inbounds ρ[ir, iphi, izi]
        if ρv <= 0.0f0
            t += ds
            continue
        end

        j_sca = include_scatter == Int32(1) ? ρv * κ_sca * (@inbounds S_sca[ir, iphi, izi]) : 0.0f0
        j_th  = include_thermal == Int32(1) ? ρv * κ_abs * (@inbounds B_nu_r[ir])           : 0.0f0
        j_tot = j_sca + j_th

        χv = ρv * κ_ext
        dτ = χv * ds
        # formal solution on the step: I += S (1 - exp(-dτ)) exp(-τ), S = j/χ
        S_step = j_tot / max(χv, 1.0f-30)
        I += S_step * (1.0f0 - exp(-dτ)) * exp(-τ)
        τ += dτ

        if τ > 30.0f0
            break
        end
        t += ds
    end

    @inbounds img[jx + Int32(1), jy + Int32(1)] = I
    return
end

"""
    render_image(grid, ρ_d, S_sca_d, obs; κ_abs, κ_sca, λ_um, T_mid_r=nothing,
                 include_thermal=true, include_scatter=true, threads=256) -> CuArray{Float32,2}

Ray-trace the disk at the given observer orientation. `S_sca_d` is the physical
scattering source function in cgs (erg s^-1 cm^-2 sr^-1). `λ_um` is the imaging
wavelength in μm; if `include_thermal` is true, `T_mid_r` (length N_r, K) must be
provided and the Planck function B_ν(T_mid(r), λ) is precomputed per radial bin.

Returns a `(N_px, N_py)` CuArray of specific intensity per pixel (erg s^-1 cm^-2 sr^-1).
"""
function render_image(grid::CylGrid{Float32}, ρ_d::CuArray{Float32,3},
                      S_sca_d::CuArray{Float32,3}, obs::ObserverParams;
                      κ_abs::Real, κ_sca::Real, λ_um::Real,
                      T_mid_r::Union{Nothing,AbstractVector}=nothing,
                      include_thermal::Bool=true, include_scatter::Bool=true,
                      threads::Integer=256)

    CUDA.allowscalar(false)

    # Precompute B_ν(T, ν) per radial bin, on host, then upload.
    λ_cm = Float64(λ_um) * 1e-4
    ν    = Constants.c_light / λ_cm
    B_nu_r_host = if include_thermal
        @assert T_mid_r !== nothing "T_mid_r required when include_thermal=true"
        Float32[ Float32(Constants.planck_B_nu(T, ν)) for T in T_mid_r ]
    else
        zeros(Float32, grid.N_r)
    end
    B_nu_r_d = CuArray(B_nu_r_host)
    log_r_edges_d = CuArray(grid.log_r_edges)

    # Observer basis (sky-x, sky-y orthonormal to d_obs)
    i = Float64(obs.inclination)
    φ = Float64(obs.azimuth)
    d_obs = (sin(i)*cos(φ), sin(i)*sin(φ), cos(i))
    # ê_x_sky: perpendicular to d_obs, in disk xy-plane (when inclination != 0)
    ex_vec = (-sin(φ), cos(φ), 0.0)
    # ê_y_sky = d_obs × ê_x_sky
    ey_vec = (d_obs[2]*ex_vec[3] - d_obs[3]*ex_vec[2],
              d_obs[3]*ex_vec[1] - d_obs[1]*ex_vec[3],
              d_obs[1]*ex_vec[2] - d_obs[2]*ex_vec[1])

    img = CUDA.zeros(Float32, obs.N_px, obs.N_py)
    L0  = Float32(2.0 * grid.r_max)

    total = obs.N_px * obs.N_py
    nblocks = cld(Int(total), threads)

    @cuda threads=threads blocks=nblocks _rt_kernel!(
        img,
        ρ_d, S_sca_d, B_nu_r_d,
        log_r_edges_d,
        grid.N_r, grid.N_phi, grid.N_z,
        Float32(grid.r_min), Float32(grid.r_max),
        Float32(grid.z_min), Float32(grid.z_max),
        Float32(grid.dphi),  Float32(grid.dz),
        Float32(κ_abs), Float32(κ_sca),
        Int32(include_thermal ? 1 : 0), Int32(include_scatter ? 1 : 0),
        Float32(d_obs[1]), Float32(d_obs[2]), Float32(d_obs[3]),
        Float32(ex_vec[1]), Float32(ex_vec[2]), Float32(ex_vec[3]),
        Float32(ey_vec[1]), Float32(ey_vec[2]), Float32(ey_vec[3]),
        obs.N_px, obs.N_py, obs.fov_cm,
        obs.n_steps, L0)

    CUDA.synchronize()
    return img
end

end # module
