"""
Optional matplotlib helpers. Require `pip install jisk[plot]` (or just
`pip install matplotlib`).
"""

from __future__ import annotations
from typing import Optional
import numpy as np

from .data import Image, SourceFunction
from .profile import AU_CM


def _import_mpl():
    try:
        import matplotlib.pyplot as plt
        return plt
    except ImportError as e:
        raise ImportError(
            "matplotlib is required for plotting. "
            "Install with `pip install matplotlib` or `pip install jisk[plot]`."
        ) from e


def plot_image(img: Image, *, log: bool = True, vmin: Optional[float] = None,
               vmax: Optional[float] = None, cmap: str = "inferno",
               ax=None, cbar: bool = True, title: Optional[str] = None):
    """Render an `Image` with matplotlib. Returns (fig, ax, im)."""
    plt = _import_mpl()

    data = np.asarray(img.image.T)   # match "sky" axes: x → right, y → up
    if log:
        data = np.log10(np.clip(data, 1e-30, None))

    extent = [-img.fov_au/2, img.fov_au/2, -img.fov_au/2, img.fov_au/2]

    if ax is None:
        fig, ax = plt.subplots(figsize=(6, 5.5))
    else:
        fig = ax.figure
    im = ax.imshow(data, origin="lower", extent=extent, cmap=cmap,
                   vmin=vmin, vmax=vmax, interpolation="nearest")
    ax.set_xlabel("sky x [AU]")
    ax.set_ylabel("sky y [AU]")
    t = title if title is not None else (
        f"i = {img.inclination_deg:.1f}°, φ = {img.azimuth_deg:.1f}°"
        + (f", λ = {img.metadata.get('wavelength_um')} μm"
           if "wavelength_um" in img.metadata else "")
    )
    ax.set_title(t)
    if cbar:
        fig.colorbar(im, ax=ax, label=("log10 I [cgs]" if log else "I [cgs]"))
    return fig, ax, im


def plot_source_slice(src: SourceFunction, *,
                      phi_index: Optional[int] = None,
                      log: bool = True, cmap: str = "viridis",
                      ax=None, cbar: bool = True):
    """Plot an (r, z) slice of the scattering source function.

    By default (`phi_index=None`) averages over the azimuthal axis — appropriate
    for the axisymmetric Chiang & Goldreich disk and dramatically reduces MC noise
    (the per-bin Poisson noise scales as 1/√N_φ when averaging).

    Pass an integer `phi_index` to view a single φ-bin instead.
    """
    plt = _import_mpl()

    if phi_index is None:
        S = np.asarray(src.S_sca).mean(axis=1)      # (N_r, N_z)
        title = f"S_sca, φ-averaged (N_φ = {src.grid.N_phi})"
    else:
        S = np.asarray(src.S_sca[:, int(phi_index), :])
        title = f"S_sca at φ-bin {int(phi_index)}"

    if log:
        S = np.log10(np.clip(S, 1e-30, None))

    if ax is None:
        fig, ax = plt.subplots(figsize=(7, 4))
    else:
        fig = ax.figure
    r_edges = src.grid.r_edges / AU_CM
    z_edges = src.grid.z_edges / AU_CM
    pc = ax.pcolormesh(r_edges, z_edges, S.T, cmap=cmap, shading="auto")
    ax.set_xlabel("r [AU]")
    ax.set_ylabel("z [AU]")
    ax.set_title(title)
    if cbar:
        fig.colorbar(pc, ax=ax, label=("log10 S_sca [cgs]" if log else "S_sca [cgs]"))
    return fig, ax, pc
