"""
jisk — Python wrapper for the Julia/CUDA.jl protoplanetary-disk
radiative-transfer module.

Entry points mirror the radmc3dPy style:

    import jisk as qd

    # Run the Monte Carlo engine (writes HDF5, returns loaded SourceFunction)
    src = qd.run_mc(wavelength=1.0, npackets=int(1e7))

    # Ray-trace at an arbitrary observer orientation
    img = src.render(inclination_deg=60, azimuth_deg=0, res=512, fov_au=250)
    img.plot()

    # Or drive the ray-tracer standalone from a saved source function
    img = qd.render_image(source="outputs/S_sca.h5", inclination_deg=60)

    # Analytic disk profile (mirrors DiskProfile.jl)
    T_K  = qd.midplane_temperature(r_au=50)
    rho  = qd.density_cyl(r_au=50, z_au=1)
"""

from .driver       import run_mc, render_image, set_julia_binary
from .data         import SourceFunction, AbsorbedEnergy, Image
from .profile      import (
    midplane_temperature, surface_density, scale_height, density_cyl,
    STAR_MASS_G, STAR_RADIUS_CM, STAR_TEMPERATURE_K, STAR_LUMINOSITY_ERGS,
    R_IN_AU, R_OUT_AU, SIGMA_D0, P_SIG, ALPHA_GRAZE, AU_CM,
)
from .plot         import plot_image, plot_source_slice
from .viz3d        import plot_3d

__version__ = "0.1.0"

__all__ = [
    "run_mc", "render_image", "set_julia_binary",
    "SourceFunction", "AbsorbedEnergy", "Image",
    "midplane_temperature", "surface_density", "scale_height", "density_cyl",
    "plot_image", "plot_source_slice", "plot_3d",
    "STAR_MASS_G", "STAR_RADIUS_CM", "STAR_TEMPERATURE_K", "STAR_LUMINOSITY_ERGS",
    "R_IN_AU", "R_OUT_AU", "SIGMA_D0", "P_SIG", "ALPHA_GRAZE", "AU_CM",
]
