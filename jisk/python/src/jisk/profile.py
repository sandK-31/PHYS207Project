"""
Analytic Chiang & Goldreich (1997) disk profile in Python.

These mirror the formulas in the Julia `DiskProfile` module. Useful for quick
overlays, validation, and setting up parameter sweeps without spawning Julia.
"""

from __future__ import annotations
import numpy as np

# Physical constants (cgs) ---------------------------------------------------
G_CGS         = 6.6743e-8
K_B           = 1.380649e-16
M_P           = 1.67262e-24
SIGMA_SB      = 5.670374e-5
AU_CM         = 1.495978707e13
M_SUN_G       = 1.98892e33
R_SUN_CM      = 6.96e10

# Stellar parameters (match Constants.jl) -----------------------------------
STAR_MASS_G          = 1.0 * M_SUN_G
STAR_RADIUS_CM       = 1.0 * R_SUN_CM
STAR_TEMPERATURE_K   = 5780.0
STAR_LUMINOSITY_ERGS = 4 * np.pi * STAR_RADIUS_CM**2 * SIGMA_SB * STAR_TEMPERATURE_K**4

# Disk parameters -----------------------------------------------------------
ALPHA_GRAZE = 0.05
SIGMA_D0    = 0.1        # g/cm^2 at 1 AU
P_SIG       = -1.0
MU_GAS      = 2.3
R_IN_AU     = 10.0
R_OUT_AU    = 500.0


def midplane_temperature(r_au=None, r_cm=None, *, alpha=None):
    """Chiang & Goldreich passive grazing-angle T_mid(r). Accepts r in AU or cm.

    `alpha` overrides the module-level `ALPHA_GRAZE` for this call.
    """
    r = _r_cm(r_au, r_cm)
    a = ALPHA_GRAZE if alpha is None else float(alpha)
    F_irr = a * STAR_LUMINOSITY_ERGS / (4 * np.pi * r**2)
    return (F_irr / SIGMA_SB) ** 0.25


def surface_density(r_au=None, r_cm=None):
    """Power-law dust surface density Σ_d(r) [g/cm^2]."""
    r = _r_cm(r_au, r_cm)
    return SIGMA_D0 * (r / AU_CM) ** P_SIG


def scale_height(r_au=None, r_cm=None, *, alpha=None):
    """Pressure scale height h(r) = c_s / Ω_k [cm].

    `alpha` overrides the module-level `ALPHA_GRAZE` for this call.
    """
    r   = _r_cm(r_au, r_cm)
    T   = midplane_temperature(r_cm=r, alpha=alpha)
    c_s = np.sqrt(K_B * T / (MU_GAS * M_P))
    Ω_k = np.sqrt(G_CGS * STAR_MASS_G / r**3)
    return c_s / Ω_k


def density_cyl(r_au=None, z_au=None, r_cm=None, z_cm=None, *,
                r_in_au=None, r_out_au=None, alpha=None):
    """
    Evaluate the Chiang & Goldreich ρ_d(r_cyl, z) [g/cm^3] at cylindrical coords.

    Uses the spherical-coordinate formula in the original model and converts on
    the fly: r_sph = √(r_cyl² + z²), θ = atan2(r_cyl, z). Zero outside
    [r_in_au, r_out_au]. Optional kwargs `r_in_au`, `r_out_au`, `alpha` override
    the module-level defaults for this call.
    """
    r  = _r_cm(r_au, r_cm)
    z  = _coerce(z_au, z_cm)
    r  = np.asarray(r, dtype=float)
    z  = np.asarray(z, dtype=float)
    rin  = R_IN_AU  if r_in_au  is None else float(r_in_au)
    rout = R_OUT_AU if r_out_au is None else float(r_out_au)

    r_sph = np.sqrt(r**2 + z**2)
    theta = np.arctan2(r, z)
    h     = scale_height(r_cm=r_sph, alpha=alpha)
    Sigma = surface_density(r_cm=r_sph)
    arg   = (np.pi/2 - theta) / (h / r_sph)
    rho   = Sigma / (np.sqrt(2*np.pi) * h) * np.exp(-0.5 * arg**2)
    inside = (r_sph >= rin * AU_CM) & (r_sph <= rout * AU_CM)
    return np.where(inside, rho, 0.0)


# --- helpers ---------------------------------------------------------------

def _r_cm(r_au, r_cm):
    if r_cm is not None and r_au is not None:
        raise TypeError("specify r_au OR r_cm, not both")
    if r_cm is not None:
        return np.asarray(r_cm, dtype=float)
    if r_au is not None:
        return np.asarray(r_au, dtype=float) * AU_CM
    raise TypeError("must provide r_au= or r_cm=")


def _coerce(a_au, a_cm):
    if a_cm is not None and a_au is not None:
        raise TypeError("specify *_au OR *_cm, not both")
    if a_cm is not None:
        return np.asarray(a_cm, dtype=float)
    if a_au is not None:
        return np.asarray(a_au, dtype=float) * AU_CM
    raise TypeError("must provide *_au= or *_cm=")
