"""
Interactive 3D visualization (Plotly). Optional extra — requires
`pip install jisk[viz3d]` or `pip install plotly`.

Writes a standalone HTML you can open in any browser; rotate / zoom / pan work
out of the box.

Three display modes:
- `mode="volume"` (default) — semi-transparent volume render. Resamples the
  cylindrical grid onto a Cartesian cube and shows continuous 3D structure.
- `mode="isosurface"` — shaded iso-contours at a handful of levels.
- `mode="scatter"` — marker per above-threshold voxel (fast, but looks spoky
  on a cylindrical grid because it shows voxel centers).
"""

from __future__ import annotations
from pathlib import Path
from typing import Optional, Union
import numpy as np

from .data import SourceFunction, AbsorbedEnergy, CylGridEdges
from .profile import AU_CM


def _import_plotly():
    try:
        import plotly.graph_objects as go
        return go
    except ImportError as e:
        raise ImportError(
            "plotly is required for plot_3d. "
            "Install with `pip install plotly` or `pip install jisk[viz3d]`."
        ) from e


def _coerce_field(field):
    """Accept SourceFunction / AbsorbedEnergy and return (values, grid, label)."""
    if isinstance(field, SourceFunction):
        return np.asarray(field.S_sca), field.grid, "S_sca"
    if isinstance(field, AbsorbedEnergy):
        return np.asarray(field.E_abs), field.grid, "E_abs"
    raise TypeError(
        "plot_3d expects a SourceFunction or AbsorbedEnergy. "
        "Pass a raw array via `plot_3d_array(values, grid=...)` if needed."
    )


def _resample_to_cartesian(values: np.ndarray, grid: CylGridEdges,
                           N: int) -> tuple:
    """
    Nearest-neighbor resample the cylindrical field onto a cubic Cartesian grid.

    Returns X, Y, Z, V — all shape (N, N, N) in cm.
    Points outside the grid (inner hole, beyond r_max, outside z range) → 0.
    """
    r_max = float(grid.r_edges[-1])
    z_lo  = float(grid.z_edges[0])
    z_hi  = float(grid.z_edges[-1])

    x = np.linspace(-r_max, r_max, N)
    y = np.linspace(-r_max, r_max, N)
    z = np.linspace( z_lo,  z_hi,  N)
    X, Y, Z = np.meshgrid(x, y, z, indexing="ij")

    R   = np.sqrt(X*X + Y*Y)
    PHI = np.arctan2(Y, X) % (2*np.pi)

    # searchsorted with 'right' gives the bin the point falls into
    ir    = np.searchsorted(grid.r_edges,   R,   side="right") - 1
    iphi  = np.searchsorted(grid.phi_edges, PHI, side="right") - 1
    iz    = np.searchsorted(grid.z_edges,   Z,   side="right") - 1

    valid = (
        (ir   >= 0) & (ir   < grid.N_r) &
        (iphi >= 0) & (iphi < grid.N_phi) &
        (iz   >= 0) & (iz   < grid.N_z)
    )
    ir   = np.clip(ir,   0, grid.N_r   - 1)
    iphi = np.clip(iphi, 0, grid.N_phi - 1)
    iz   = np.clip(iz,   0, grid.N_z   - 1)

    V = np.where(valid, values[ir, iphi, iz], 0.0)
    return X, Y, Z, V


def plot_3d(
    field: Union[SourceFunction, AbsorbedEnergy],
    *,
    mode: str = "volume",
    log: bool = True,
    floor_abs: float = 1e-30,
    cmap: str = "Inferno",
    units: str = "AU",
    title: Optional[str] = None,
    out: Union[str, Path, None] = None,
    show: bool = True,

    # ---- volume / isosurface controls -------------------------------------
    N_cart: int = 64,
    color_percentiles: tuple = (2.0, 99.0),
    opacity: float = 0.1,
    surface_count: int = 17,
    opacityscale: str = "max",
    iso_levels: Optional[list] = None,

    # ---- scatter controls -------------------------------------------------
    threshold: Optional[float] = None,
    threshold_percentile: float = 20.0,
    max_points: int = 80_000,
    marker_size: float = 2.2,
    scatter_opacity: float = 0.55,
):
    """
    Interactive 3D render of a cylindrical-grid scalar field.

    Parameters
    ----------
    field : SourceFunction | AbsorbedEnergy
    mode : {"volume", "isosurface", "scatter"}
        Rendering method. `volume` is the default — resamples to a Cartesian
        cube and shows continuous structure with semi-transparent shading.
    log, floor_abs : bool, float
        Log10 stretch with an absolute floor applied before display.
    cmap : str
        Any Plotly colorscale name.
    units : {"AU", "cm"}
    out : path, optional
        Path to write a standalone HTML file (extension auto-set to `.html`).
    show : bool
        Try to open in a browser if no `out` is given.

    Volume/isosurface-specific
    --------------------------
    N_cart : int
        Cartesian resampling resolution (N_cart^3 voxels). Default 64 (262 k
        voxels) — usable in any browser. 96 (884 k) is near the practical
        ceiling for `go.Volume` in Plotly before browsers start to freeze.
    color_percentiles : (float, float)
        Color-scale clip percentiles on the in-range values.
    opacity : float
        Overall volume opacity multiplier (volume mode).
    surface_count : int
        Number of implicit iso-shells Plotly stacks (volume mode).
    opacityscale : {"max", "uniform", "min", "extremes"} or list
        How opacity varies across the value range.
    iso_levels : list of float, optional
        Explicit iso-values for `mode="isosurface"` (in log10 space if `log`).
        Defaults to three levels at the 60th / 80th / 95th percentiles.

    Scatter-specific
    ----------------
    threshold, threshold_percentile, max_points, marker_size, scatter_opacity.

    Returns
    -------
    plotly.graph_objects.Figure
    """
    go = _import_plotly()
    values, grid, label = _coerce_field(field)
    if values.shape != (grid.N_r, grid.N_phi, grid.N_z):
        raise ValueError(
            f"shape mismatch: values {values.shape} vs grid "
            f"({grid.N_r}, {grid.N_phi}, {grid.N_z})"
        )
    scale = AU_CM if units == "AU" else 1.0
    suffix = " [AU]" if units == "AU" else " [cm]"

    if mode in ("volume", "isosurface"):
        fig = _plot_structured(
            values, grid, label, log=log, floor_abs=floor_abs, cmap=cmap,
            N_cart=N_cart, color_percentiles=color_percentiles,
            opacity=opacity, surface_count=surface_count,
            opacityscale=opacityscale, iso_levels=iso_levels,
            scale=scale, suffix=suffix, title=title, mode=mode, go=go,
        )
    elif mode == "scatter":
        fig = _plot_scatter(
            values, grid, label, log=log, floor_abs=floor_abs, cmap=cmap,
            threshold=threshold, threshold_percentile=threshold_percentile,
            max_points=max_points, marker_size=marker_size,
            opacity=scatter_opacity, color_percentiles=color_percentiles,
            scale=scale, suffix=suffix, title=title, go=go,
        )
    else:
        raise ValueError(f"unknown mode={mode!r}; use 'volume', 'isosurface', or 'scatter'")

    if out is not None:
        out = Path(out)
        if out.suffix.lower() != ".html":
            out = out.with_suffix(".html")
        out.parent.mkdir(parents=True, exist_ok=True)
        # Embed plotly.js inline so the HTML opens from file:// without network
        # (CDN links often fail to load in local-file contexts). Adds ~3 MB.
        fig.write_html(str(out), include_plotlyjs="inline", full_html=True)
        print(f"wrote {out}  ({out.stat().st_size / 1e6:.1f} MB)")

    if show and out is None:
        try:
            fig.show()
        except Exception:
            pass

    return fig


# ---- volume / isosurface --------------------------------------------------

def _plot_structured(values, grid, label, *, log, floor_abs, cmap,
                     N_cart, color_percentiles, opacity, surface_count,
                     opacityscale, iso_levels, scale, suffix, title, mode, go):
    X, Y, Z, V = _resample_to_cartesian(values, grid, int(N_cart))

    # Apply log stretch after resampling so zeros map to the floor
    if log:
        V_disp = np.log10(np.clip(V, floor_abs, None))
        cbar_title = f"log10 {label}"
    else:
        V_disp = V.copy()
        cbar_title = label

    # Color scale set from the in-disk voxels only (exclude the zero background)
    in_disk = V > 0
    if in_disk.any():
        vals_in = V_disp[in_disk]
        cmin = float(np.quantile(vals_in, max(0.0, color_percentiles[0]) / 100.0))
        cmax = float(np.quantile(vals_in, min(100.0, color_percentiles[1]) / 100.0))
    else:
        cmin, cmax = 0.0, 1.0
    if not (cmax > cmin):
        cmax = cmin + 1.0

    xs = (X / scale).ravel()
    ys = (Y / scale).ravel()
    zs = (Z / scale).ravel()
    vs = V_disp.ravel()

    if mode == "volume":
        trace = go.Volume(
            x = xs, y = ys, z = zs, value = vs,
            isomin = cmin, isomax = cmax,
            opacity = float(opacity),
            opacityscale = opacityscale,
            surface_count = int(surface_count),
            colorscale = cmap,
            cmin = cmin, cmax = cmax,
            colorbar = dict(title=cbar_title),
            showscale = True,
            caps = dict(x_show=False, y_show=False, z_show=False),
        )
        mode_label = "volume"
    else:  # isosurface
        if iso_levels is None:
            if in_disk.any():
                iso_levels = [
                    float(np.quantile(vals_in, q))
                    for q in (0.60, 0.80, 0.95)
                ]
            else:
                iso_levels = [cmin, 0.5 * (cmin + cmax), cmax]
        iso_levels = sorted(set(iso_levels))
        trace = go.Isosurface(
            x = xs, y = ys, z = zs, value = vs,
            isomin = iso_levels[0], isomax = iso_levels[-1],
            surface_count = len(iso_levels),
            opacity = max(0.2, float(opacity) * 4),
            colorscale = cmap,
            cmin = cmin, cmax = cmax,
            colorbar = dict(title=cbar_title),
            showscale = True,
            caps = dict(x_show=False, y_show=False, z_show=False),
        )
        mode_label = "isosurface"

    fig = go.Figure(data=[trace])

    r_max_disp = float(grid.r_edges[-1]) / scale
    z_lo_disp  = float(grid.z_edges[0])  / scale
    z_hi_disp  = float(grid.z_edges[-1]) / scale
    fig.update_layout(
        title = title if title is not None else (
            f"{label} — {mode_label} render "
            f"({N_cart}³ Cartesian resample, "
            f"{int(in_disk.sum())} in-disk voxels)"
        ),
        scene = dict(
            xaxis = dict(title="x" + suffix, range=[-r_max_disp, r_max_disp]),
            yaxis = dict(title="y" + suffix, range=[-r_max_disp, r_max_disp]),
            zaxis = dict(title="z" + suffix, range=[z_lo_disp, z_hi_disp]),
            aspectmode = "data",
        ),
        margin = dict(l=0, r=0, t=40, b=0),
    )
    return fig


# ---- scatter (kept as a fast fallback) -----------------------------------

def _plot_scatter(values, grid, label, *, log, floor_abs, cmap,
                  threshold, threshold_percentile, max_points, marker_size,
                  opacity, color_percentiles, scale, suffix, title, go):
    r_c   = 0.5 * (grid.r_edges[1:]   + grid.r_edges[:-1])
    phi_c = 0.5 * (grid.phi_edges[1:] + grid.phi_edges[:-1])
    z_c   = 0.5 * (grid.z_edges[1:]   + grid.z_edges[:-1])
    R, PHI, Z = np.meshgrid(r_c, phi_c, z_c, indexing="ij")
    X = R * np.cos(PHI)
    Y = R * np.sin(PHI)

    finite_positive = values[np.isfinite(values) & (values > 0)]
    if finite_positive.size == 0:
        raise ValueError("field has no positive finite values; nothing to plot.")

    if threshold is None:
        pct = max(0.0, min(100.0, float(threshold_percentile)))
        threshold = float(np.quantile(finite_positive, pct / 100.0)) if pct > 0 else 0.0

    mask = np.isfinite(values) & (values > threshold)
    n = int(mask.sum())
    if n == 0:
        raise ValueError(
            f"no voxels exceed threshold={threshold:.3g}; "
            "lower `threshold_percentile` or pass `threshold=0`."
        )

    xs = X[mask].ravel()
    ys = Y[mask].ravel()
    zs = Z[mask].ravel()
    vs = values[mask].ravel().astype(float)

    if n > max_points:
        idx = np.random.default_rng(0).choice(n, size=max_points, replace=False)
        xs, ys, zs, vs = xs[idx], ys[idx], zs[idx], vs[idx]

    if log:
        color = np.log10(np.clip(vs, floor_abs, None))
        cbar_title = f"log10 {label}"
    else:
        color = vs
        cbar_title = label
    cmin = float(np.quantile(color, max(0.0, color_percentiles[0]) / 100.0))
    cmax = float(np.quantile(color, min(100.0, color_percentiles[1]) / 100.0))
    if not (cmax > cmin):
        cmax = cmin + 1.0

    fig = go.Figure(data=[go.Scatter3d(
        x = xs / scale, y = ys / scale, z = zs / scale,
        mode = "markers",
        marker = dict(
            size = marker_size, color = color,
            colorscale = cmap, cmin = cmin, cmax = cmax,
            opacity = opacity,
            colorbar = dict(title=cbar_title),
        ),
        name = label,
    )])

    r_max_disp = float(grid.r_edges[-1]) / scale
    z_lo_disp  = float(grid.z_edges[0])  / scale
    z_hi_disp  = float(grid.z_edges[-1]) / scale
    fig.update_layout(
        title = title if title is not None else (
            f"{label} — scatter (n={n} above threshold = {threshold:.2g}"
            + (f", showing {max_points})" if n > max_points else ")")
        ),
        scene = dict(
            xaxis = dict(title="x" + suffix, range=[-r_max_disp, r_max_disp]),
            yaxis = dict(title="y" + suffix, range=[-r_max_disp, r_max_disp]),
            zaxis = dict(title="z" + suffix, range=[z_lo_disp, z_hi_disp]),
            aspectmode = "data",
        ),
        margin = dict(l=0, r=0, t=40, b=0),
    )
    return fig
