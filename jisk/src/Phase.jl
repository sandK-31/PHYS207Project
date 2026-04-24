module Phase

export sample_isotropic, sample_hg, rotate_to_frame

"""
    sample_isotropic(ξ1, ξ2) -> (ux, uy, uz)

Uniform direction on the unit sphere from two U(0,1) samples.
"""
@inline function sample_isotropic(ξ1::T, ξ2::T) where {T}
    μ = T(2) * ξ1 - T(1)                # cos θ
    s = sqrt(max(T(0), T(1) - μ*μ))
    φ = T(2π) * ξ2
    return s*cos(φ), s*sin(φ), μ
end

"""
    sample_hg(g, ξ) -> μ

Sample cos(scattering angle) from the Henyey–Greenstein phase function with
asymmetry parameter g ∈ (-1, 1). For |g| < 1e-3 falls back to isotropic.
"""
@inline function sample_hg(g::T, ξ::T) where {T}
    if abs(g) < T(1e-3)
        return T(2)*ξ - T(1)
    end
    s = (T(1) - g*g) / (T(1) - g + T(2)*g*ξ)
    μ = (T(1) + g*g - s*s) / (T(2)*g)
    return max(T(-1), min(T(1), μ))
end

"""
    rotate_to_frame(ux, uy, uz, μ, φ) -> (ux', uy', uz')

Given the current direction (ux, uy, uz) and a scattering (μ = cosθ_s, φ_s) in
the local frame, return the new world-frame direction. Uses the classic
polar-rotation formulas to avoid gimbal issues near uz = ±1.
"""
@inline function rotate_to_frame(ux::T, uy::T, uz::T, μ::T, φ::T) where {T}
    sθ = sqrt(max(T(0), T(1) - μ*μ))
    cφ = cos(φ)
    sφ = sin(φ)

    if abs(uz) > T(0.99999)
        sign_uz = uz >= T(0) ? T(1) : T(-1)
        return sθ * cφ, sign_uz * sθ * sφ, sign_uz * μ
    end

    denom = sqrt(max(T(1e-30), T(1) - uz*uz))
    ux_new = sθ * (ux*uz*cφ - uy*sφ) / denom + ux * μ
    uy_new = sθ * (uy*uz*cφ + ux*sφ) / denom + uy * μ
    uz_new = -sθ * cφ * denom + uz * μ

    norm = sqrt(ux_new*ux_new + uy_new*uy_new + uz_new*uz_new)
    return ux_new/norm, uy_new/norm, uz_new/norm
end

end # module
