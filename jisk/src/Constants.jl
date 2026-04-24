module Constants

const G         = 6.6743e-8              # cm^3 g^-1 s^-2
const k_B       = 1.380649e-16            # erg / K
const m_p       = 1.67262e-24             # g
const sigma_SB  = 5.670374e-5             # erg cm^-2 s^-1 K^-4
const c_light   = 2.99792458e10           # cm / s
const h_planck  = 6.62607015e-27          # erg s
const AU        = 1.495978707e13          # cm
const M_sun     = 1.98892e33              # g
const R_sun     = 6.96e10                 # cm

const T_star    = 5780.0                  # K
const M_star    = 1.0 * M_sun
const R_star    = 1.0 * R_sun
const L_star    = 4π * R_star^2 * sigma_SB * T_star^4

const mu_gas    = 2.3
const alpha_graze = 0.05
const Sigma_d0  = 0.1                     # g / cm^2 at 1 AU
const p_sig     = -1.0

const R_in_AU   = 10.0
const R_out_AU  = 100.0

@inline function planck_B_nu(T::Real, nu::Real)
    x = h_planck * nu / (k_B * T)
    x > 80.0f0 && return zero(typeof(x))
    return (2 * h_planck * nu^3 / c_light^2) / (exp(x) - 1)
end

@inline function planck_B_lambda_cgs(T::Real, lambda_cm::Real)
    x = h_planck * c_light / (lambda_cm * k_B * T)
    x > 80.0f0 && return zero(typeof(x))
    return (2 * h_planck * c_light^2 / lambda_cm^5) / (exp(x) - 1)
end

end # module
