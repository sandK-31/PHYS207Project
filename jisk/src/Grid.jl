module Grid

using ..Constants

export CylGrid, build_grid, voxel_volume_array

struct CylGrid{T<:Real}
    N_r::Int32
    N_phi::Int32
    N_z::Int32
    r_edges::Vector{T}       # length N_r+1, cm
    log_r_edges::Vector{T}   # log(r_edges)
    phi_edges::Vector{T}     # length N_phi+1, rad, [0, 2π]
    z_edges::Vector{T}       # length N_z+1, cm
    r_min::T
    r_max::T
    z_min::T
    z_max::T
    dphi::T
    dz::T
end

function build_grid(; N_r::Integer=128, N_phi::Integer=64, N_z::Integer=128,
                      r_min_AU::Real=Constants.R_in_AU,
                      r_max_AU::Real=Constants.R_out_AU,
                      z_max_AU::Real=5.0,
                      T::Type=Float32)
    r_min = T(r_min_AU * Constants.AU)
    r_max = T(r_max_AU * Constants.AU)
    z_max = T(z_max_AU * Constants.AU)
    z_min = -z_max

    log_edges = collect(range(log(r_min), log(r_max); length=N_r+1))
    r_edges   = exp.(log_edges)
    phi_edges = collect(range(T(0), T(2π); length=N_phi+1))
    z_edges   = collect(range(z_min, z_max; length=N_z+1))

    return CylGrid{T}(
        Int32(N_r), Int32(N_phi), Int32(N_z),
        T.(r_edges), T.(log_edges), T.(phi_edges), T.(z_edges),
        r_min, r_max, z_min, z_max,
        T(2π / N_phi), T((z_max - z_min) / N_z),
    )
end

"""
    voxel_volume_array(grid; T=Float64)

Return a 3D CPU array of voxel volumes (cm^3), shape (N_r, N_phi, N_z).
V = (1/2)(r_{i+1}^2 - r_i^2) · Δφ · Δz.

Defaults to Float64 because voxel volumes in the disk reach ~10^40 cm^3, which
exceeds the Float32 dynamic range. Caller can request Float32 explicitly if
they know the grid is small enough.
"""
function voxel_volume_array(grid::CylGrid; T::Type=Float64)
    V = Array{T}(undef, grid.N_r, grid.N_phi, grid.N_z)
    dz   = T(grid.dz)
    dphi = T(grid.dphi)
    @inbounds for k in 1:grid.N_z, j in 1:grid.N_phi, i in 1:grid.N_r
        r_lo = T(grid.r_edges[i])
        r_hi = T(grid.r_edges[i+1])
        V[i, j, k] = T(0.5) * (r_hi*r_hi - r_lo*r_lo) * dphi * dz
    end
    return V
end

"""
    index_r(r, log_r_edges, N_r)

Branchless binary search: return 1-based radial bin index, or 0 if r is outside the grid.
Device-compatible (works inside CUDA kernels).
"""
@inline function index_r(r::T, log_r_edges, N_r::Integer) where {T}
    r <= zero(T) && return Int32(0)
    lr = log(r)
    @inbounds (lr < log_r_edges[1] || lr >= log_r_edges[N_r + 1]) && return Int32(0)
    lo = Int32(1)
    hi = Int32(N_r + 1)
    while hi - lo > Int32(1)
        mid = (lo + hi) >> 1
        @inbounds if log_r_edges[mid] <= lr
            lo = mid
        else
            hi = mid
        end
    end
    return lo
end

"""
    index_phi(phi, dphi, N_phi)

phi assumed shifted to [0, 2π).
"""
@inline function index_phi(phi::T, dphi::T, N_phi::Integer) where {T}
    i = unsafe_trunc(Int32, phi / dphi) + Int32(1)
    i < Int32(1) && return Int32(1)
    i > Int32(N_phi) && return Int32(N_phi)
    return i
end

"""
    index_z(z, z_min, dz, N_z)
"""
@inline function index_z(z::T, z_min::T, dz::T, N_z::Integer) where {T}
    i = unsafe_trunc(Int32, (z - z_min) / dz) + Int32(1)
    (i < Int32(1) || i > Int32(N_z)) && return Int32(0)
    return i
end

"""
    cyl_indices_from_cart(x, y, z, log_r_edges, N_r, dphi, N_phi, z_min, dz, N_z)

Return (ir, iphi, iz) as Int32s. Any index == 0 signals "outside grid".
"""
@inline function cyl_indices_from_cart(x::T, y::T, z::T,
                                       log_r_edges, N_r::Integer,
                                       dphi::T, N_phi::Integer,
                                       z_min::T, dz::T, N_z::Integer) where {T}
    r = sqrt(x*x + y*y)
    phi = atan(y, x)
    phi = phi < zero(T) ? phi + T(2π) : phi
    ir   = index_r(r, log_r_edges, N_r)
    iphi = index_phi(phi, dphi, N_phi)
    iz   = index_z(z, z_min, dz, N_z)
    return ir, iphi, iz
end

end # module
