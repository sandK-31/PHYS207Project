"""
Data containers for HDF5 outputs of the Julia `jisk` pipeline.

Mirrors `radmc3dPy.data.radmc3dData` / `radmc3dImage` in spirit: each class
loads an HDF5 file written by the Julia driver scripts and exposes the arrays
as numpy arrays plus a metadata dict.
"""

from __future__ import annotations
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, Dict, Any
import numpy as np
import h5py

from .profile import AU_CM


def _from_julia_axes(a: np.ndarray) -> np.ndarray:
    """
    Reverse all axes of an array loaded from an HDF5 file written by Julia.

    Julia stores arrays in column-major order; h5py reads them back in row-major,
    which makes the dimensions appear reversed. We undo that here so Python sees
    the same logical layout as the Julia code documents:
        S_sca : (N_r, N_phi, N_z)
        image : (N_px, N_py)
    """
    return np.ascontiguousarray(a.T) if a.ndim >= 2 else a


def _read_metadata(group: h5py.Group) -> Dict[str, Any]:
    out: Dict[str, Any] = {}
    for k in group.attrs:
        v = group.attrs[k]
        try:
            v = v.item() if hasattr(v, "item") else v
        except Exception:
            pass
        if isinstance(v, bytes):
            v = v.decode("utf-8", errors="replace")
        out[k] = v
    return out


@dataclass
class CylGridEdges:
    """Edge arrays (cm) read back from an HDF5 output."""
    r_edges:   np.ndarray
    phi_edges: np.ndarray
    z_edges:   np.ndarray

    @property
    def N_r(self) -> int:   return len(self.r_edges) - 1
    @property
    def N_phi(self) -> int: return len(self.phi_edges) - 1
    @property
    def N_z(self) -> int:   return len(self.z_edges) - 1

    @property
    def r_centers_au(self) -> np.ndarray:
        return 0.5 * (self.r_edges[1:] + self.r_edges[:-1]) / AU_CM
    @property
    def phi_centers(self) -> np.ndarray:
        return 0.5 * (self.phi_edges[1:] + self.phi_edges[:-1])
    @property
    def z_centers_au(self) -> np.ndarray:
        return 0.5 * (self.z_edges[1:] + self.z_edges[:-1]) / AU_CM


@dataclass
class SourceFunction:
    """
    The 3D scattering source function S_sca(r, φ, z) produced by `run_mc`.

    Units: erg s^-1 cm^-2 sr^-1. Shape: (N_r, N_phi, N_z).
    """
    S_sca:    np.ndarray
    grid:     CylGridEdges
    metadata: Dict[str, Any] = field(default_factory=dict)
    path:     Optional[str]  = None

    @classmethod
    def load(cls, path: str | Path) -> "SourceFunction":
        path = str(Path(path))
        with h5py.File(path, "r") as f:
            S = _from_julia_axes(np.asarray(f["S_sca"]))     # → (N_r, N_phi, N_z)
            grid = CylGridEdges(
                r_edges   = np.asarray(f["grid/r_edges"]),
                phi_edges = np.asarray(f["grid/phi_edges"]),
                z_edges   = np.asarray(f["grid/z_edges"]),
            )
            metadata = _read_metadata(f["metadata"]) if "metadata" in f else {}
        return cls(S_sca=S, grid=grid, metadata=metadata, path=path)

    # convenience properties
    @property
    def r_au(self) -> np.ndarray:   return self.grid.r_centers_au
    @property
    def z_au(self) -> np.ndarray:   return self.grid.z_centers_au
    @property
    def wavelength_um(self) -> Optional[float]:
        v = self.metadata.get("wavelength_um")
        return float(v) if v is not None else None

    def render(self, **kwargs) -> "Image":
        """Shortcut: render an image from this source function via the Julia ray-tracer.

        Keyword arguments are forwarded to `jisk.render_image`.
        """
        if self.path is None:
            raise RuntimeError("SourceFunction was not loaded from disk; "
                               "`render` requires an on-disk HDF5 file. "
                               "Call `jisk.run_mc(...)` to produce one.")
        from .driver import render_image
        return render_image(source=self.path, **kwargs)


@dataclass
class AbsorbedEnergy:
    """Per-voxel absorbed packet weight from the MC. Multiply by L★/N_p for erg/s."""
    E_abs:    np.ndarray
    grid:     CylGridEdges
    metadata: Dict[str, Any] = field(default_factory=dict)
    path:     Optional[str]  = None

    @classmethod
    def load(cls, path: str | Path) -> "AbsorbedEnergy":
        path = str(Path(path))
        with h5py.File(path, "r") as f:
            E = _from_julia_axes(np.asarray(f["E_abs"]))     # → (N_r, N_phi, N_z)
            grid = CylGridEdges(
                r_edges   = np.asarray(f["grid/r_edges"]),
                phi_edges = np.asarray(f["grid/phi_edges"]),
                z_edges   = np.asarray(f["grid/z_edges"]),
            )
            metadata = _read_metadata(f["metadata"]) if "metadata" in f else {}
        return cls(E_abs=E, grid=grid, metadata=metadata, path=path)


@dataclass
class Image:
    """
    2D intensity image from the formal ray-tracer. Units: erg s^-1 cm^-2 sr^-1.
    """
    image:           np.ndarray                      # (N_px, N_py)
    inclination_rad: float
    azimuth_rad:     float
    fov_cm:          float
    n_steps:         int
    metadata:        Dict[str, Any] = field(default_factory=dict)
    path:            Optional[str]  = None

    @classmethod
    def load(cls, path: str | Path) -> "Image":
        path = str(Path(path))
        with h5py.File(path, "r") as f:
            img = _from_julia_axes(np.asarray(f["image"]))    # → (N_px, N_py)
            obs = f["observer"].attrs
            metadata = _read_metadata(f["metadata"]) if "metadata" in f else {}
            return cls(
                image           = img,
                inclination_rad = float(obs["inclination_rad"]),
                azimuth_rad     = float(obs["azimuth_rad"]),
                fov_cm          = float(obs["fov_cm"]),
                n_steps         = int(obs["n_steps"]),
                metadata        = metadata,
                path            = path,
            )

    @property
    def fov_au(self) -> float:        return self.fov_cm / AU_CM
    @property
    def inclination_deg(self) -> float: return float(np.degrees(self.inclination_rad))
    @property
    def azimuth_deg(self) -> float:     return float(np.degrees(self.azimuth_rad))
    @property
    def png_path(self) -> Optional[str]:
        """Sibling PNG that the Julia driver writes next to the HDF5, if present."""
        if self.path is None:
            return None
        p = self.path[:-3] + ".png" if self.path.endswith(".h5") else self.path + ".png"
        return p if Path(p).exists() else None
