module DiskProfile

using ..Constants
using ..Grid

export midplane_temperature, sound_speed, scale_height, surface_density,
       density_cyl, build_density_array, build_T_mid_array

"""
    midplane_temperature(r_cm; alpha=Constants.alpha_graze)

Chiang & Goldreich passive grazing-angle model:
    F_irr = α L_* / (4π r^2),   T_mid = (F_irr / σ_SB)^{1/4}.
Returns Kelvin.
"""
@inline function midplane_temperature(r_cm::Real; alpha::Real=Constants.alpha_graze)
    F_irr = alpha * Constants.L_star / (4π * r_cm^2)
    return (F_irr / Constants.sigma_SB)^(1/4)
end

@inline function sound_speed(T_mid::Real)
    return sqrt(Constants.k_B * T_mid / (Constants.mu_gas * Constants.m_p))
end

@inline function keplerian_omega(r_cm::Real)
    return sqrt(Constants.G * Constants.M_star / r_cm^3)
end

@inline function scale_height(r_cm::Real; alpha::Real=Constants.alpha_graze)
    T = midplane_temperature(r_cm; alpha=alpha)
    c_s = sound_speed(T)
    Ω_k = keplerian_omega(r_cm)
    return c_s / Ω_k
end

"""
    surface_density(r_cm)

Σ_d(r) = Σ_{d0} (r/AU)^{p_sig},  g/cm^2.
"""
@inline function surface_density(r_cm::Real)
    return Constants.Sigma_d0 * (r_cm / Constants.AU)^Constants.p_sig
end

"""
    density_cyl(r_cyl_cm, z_cm; r_in_cm, r_out_cm, alpha)

Evaluate Chiang & Goldreich ρ_d at cylindrical (r_cyl, z):
    ρ = Σ(r_sph) / (√(2π) h(r_sph)) · exp(-(π/2 - θ)^2 / 2(h/r)^2)
where r_sph, θ are spherical coordinates of the cylindrical point.
Zero outside [r_in_cm, r_out_cm].
"""
@inline function density_cyl(r_cyl::Real, z::Real;
                             r_in_cm::Real  = Constants.R_in_AU  * Constants.AU,
                             r_out_cm::Real = Constants.R_out_AU * Constants.AU,
                             alpha::Real    = Constants.alpha_graze)
    r_sph = sqrt(r_cyl*r_cyl + z*z)
    if r_sph < r_in_cm || r_sph > r_out_cm
        return zero(r_sph)
    end
    θ = atan(r_cyl, z)                # θ from +z axis; midplane at π/2
    h = scale_height(r_sph; alpha=alpha)
    Σ = surface_density(r_sph)
    arg = (π/2 - θ) / (h / r_sph)
    return Σ / (sqrt(2π) * h) * exp(-0.5 * arg * arg)
end

"""
    build_density_array(grid; r_in_au, r_out_au, alpha)

Return Array{T, 3} of ρ (g/cm^3) at voxel centers, shape (N_r, N_phi, N_z).
Density is independent of φ (axisymmetric).
"""
function build_density_array(grid::CylGrid{T};
                             r_in_au::Real  = Constants.R_in_AU,
                             r_out_au::Real = Constants.R_out_AU,
                             alpha::Real    = Constants.alpha_graze) where {T}
    r_in_cm  = Float64(r_in_au)  * Constants.AU
    r_out_cm = Float64(r_out_au) * Constants.AU
    ρ = Array{T}(undef, grid.N_r, grid.N_phi, grid.N_z)
    @inbounds for k in 1:grid.N_z
        z_c = T(0.5) * (grid.z_edges[k] + grid.z_edges[k+1])
        for i in 1:grid.N_r
            r_c = T(0.5) * (grid.r_edges[i] + grid.r_edges[i+1])
            ρ_val = T(density_cyl(Float64(r_c), Float64(z_c);
                                  r_in_cm=r_in_cm, r_out_cm=r_out_cm, alpha=alpha))
            for j in 1:grid.N_phi
                ρ[i, j, k] = ρ_val
            end
        end
    end
    return ρ
end

"""
    build_T_mid_array(grid; alpha)

Return Vector{T} of length N_r of midplane temperature at radial cell centers (K).
"""
function build_T_mid_array(grid::CylGrid{T};
                           alpha::Real=Constants.alpha_graze) where {T}
    Tm = Vector{T}(undef, grid.N_r)
    @inbounds for i in 1:grid.N_r
        r_c = T(0.5) * (grid.r_edges[i] + grid.r_edges[i+1])
        Tm[i] = T(midplane_temperature(Float64(r_c); alpha=alpha))
    end
    return Tm
end

end # module
