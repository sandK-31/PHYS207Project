"""
Subprocess drivers around the Julia `run_mc.jl` / `make_image.jl` scripts.

Users call the Python functions (`run_mc`, `render_image`) which shell out to
Julia, wait for completion, then load the HDF5 outputs back into Python
`SourceFunction` / `Image` objects.
"""

from __future__ import annotations
import os
import shutil
import subprocess
from pathlib import Path
from typing import Optional, Union

from .data import SourceFunction, Image, AbsorbedEnergy


# Resolve the Julia project (the parent of this python/ package) -----------
_PY_PKG_ROOT = Path(__file__).resolve().parent
_DEFAULT_JULIA_PROJECT = (_PY_PKG_ROOT.parents[2]).resolve()   # jisk/
_MC_SCRIPT    = _DEFAULT_JULIA_PROJECT / "scripts" / "run_mc.jl"
_IMG_SCRIPT   = _DEFAULT_JULIA_PROJECT / "scripts" / "make_image.jl"

_JULIA_BIN = os.environ.get("JISK_JULIA", "julia")


def set_julia_binary(path: str) -> None:
    """Override which `julia` executable to use (otherwise taken from $PATH or $JISK_JULIA)."""
    global _JULIA_BIN
    _JULIA_BIN = path


def _check_julia() -> str:
    found = shutil.which(_JULIA_BIN)
    if found is None:
        raise RuntimeError(
            f"Julia binary {_JULIA_BIN!r} not found in PATH. "
            f"Install Julia (https://julialang.org) or call `jisk.set_julia_binary(...)`."
        )
    return found


def _run_julia(script: Path, args: list[str],
               julia_project: Path, verbose: bool) -> None:
    julia = _check_julia()
    cmd = [julia, f"--project={julia_project}", "--startup-file=no", str(script), *args]
    if verbose:
        print("$", " ".join(cmd))
    res = subprocess.run(cmd, check=False)
    if res.returncode != 0:
        raise RuntimeError(f"Julia subprocess failed with exit code {res.returncode}: {' '.join(cmd)}")


def run_mc(
    *,
    wavelength: float = 1.0,
    npackets: int = 1_000_000,
    phase: str = "hg",
    g: Optional[float] = None,
    r_in_au: Optional[float] = None,
    r_out_au: Optional[float] = None,
    alpha_graze: Optional[float] = None,
    z_max_au: Optional[float] = None,
    Nr: int = 128,
    Nphi: int = 64,
    Nz: int = 128,
    seed: Optional[int] = None,
    max_scatter: int = 100,
    out: Union[str, Path, None] = None,
    julia_project: Union[str, Path, None] = None,
    verbose: bool = True,
    load_result: bool = True,
) -> Union[SourceFunction, None]:
    """
    Drive the Julia Monte Carlo engine.

    Parameters
    ----------
    wavelength : float
        Imaging/transport wavelength in μm (κ interpolated from ../data/opacity.dat).
    npackets : int
        Number of photon packets to launch.
    phase : {'hg', 'iso'}
        Scattering phase function.
    g : float, optional
        Henyey-Greenstein asymmetry parameter (used when phase='hg').
        If None (default), interpolated from the opacity table at `wavelength`.
        Pass an explicit float in (-1, 1) to override — e.g. `g=0.0` for an
        ~isotropic HG, `g=0.5` for strongly forward-scattering.
    r_in_au, r_out_au : float, optional
        Disk inner and outer radii in AU. Override the library defaults
        (10 / 100 AU). The grid is built between these radii and the density
        is zero outside them.
    alpha_graze : float, optional
        Grazing angle α used in the Chiang & Goldreich passive-heating
        prescription (`F_irr = α L★ / 4π r²`). Default 0.05.
        Larger α → warmer disk → larger scale height.
    z_max_au : float, optional
        Half-extent of the cylindrical grid in z (AU). Default: 5 × scale-height
        at `r_out_au` evaluated with the given `alpha_graze`. Enlarge if you
        increase α or want to capture more of the disk atmosphere.
    Nr, Nphi, Nz : int
        Cylindrical grid resolution.
    seed : int, optional
        64-bit RNG seed (defaults to Julia driver's default).
    max_scatter : int
        Scatter-count threshold before Russian roulette kicks in.
    out : str | Path, optional
        HDF5 output path for the source function. Defaults to `<project>/outputs/S_sca.h5`.
    julia_project : str | Path, optional
        Path to the Julia project (directory with Project.toml). Defaults to the
        sibling Julia module that ships with this package.
    verbose : bool
        Print the Julia invocation.
    load_result : bool
        If True (default), load the written source function and return it.

    Returns
    -------
    SourceFunction | None
    """
    proj = Path(julia_project) if julia_project else _DEFAULT_JULIA_PROJECT
    if out is None:
        out = proj / "outputs" / "S_sca.h5"
    out = Path(out).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)

    args = [
        "--wavelength",  f"{float(wavelength)}",
        "--npackets",    f"{int(npackets)}",
        "--phase",       str(phase),
        "--Nr",          str(int(Nr)),
        "--Nphi",        str(int(Nphi)),
        "--Nz",          str(int(Nz)),
        "--max-scatter", str(int(max_scatter)),
        "--out",         str(out),
    ]
    if seed is not None:
        args += ["--seed", str(int(seed))]
    if g is not None:
        args += ["--g", f"{float(g)}"]
    if r_in_au is not None:
        args += ["--r-in-au", f"{float(r_in_au)}"]
    if r_out_au is not None:
        args += ["--r-out-au", f"{float(r_out_au)}"]
    if alpha_graze is not None:
        args += ["--alpha", f"{float(alpha_graze)}"]
    if z_max_au is not None:
        args += ["--z-max-au", f"{float(z_max_au)}"]

    _run_julia(_MC_SCRIPT, args, proj, verbose)

    if not load_result:
        return None
    return SourceFunction.load(out)


def render_image(
    *,
    source: Union[str, Path, "SourceFunction"],
    inclination_deg: float = 60.0,
    azimuth_deg: float = 0.0,
    res: int = 512,
    fov_au: float = 250.0,
    nsteps: int = 512,
    wavelength: Optional[float] = None,
    include_thermal: bool = True,
    include_scatter: bool = True,
    png: bool = True,
    cmap: str = "inferno",
    log_png: bool = True,
    out: Union[str, Path, None] = None,
    julia_project: Union[str, Path, None] = None,
    verbose: bool = True,
    load_result: bool = True,
) -> Union[Image, None]:
    """
    Drive the Julia formal ray-tracer on a saved scattering source function.

    `source` may be either a path to an `S_sca.h5` file or a loaded
    `SourceFunction` instance (in which case `.path` is used).
    """
    proj = Path(julia_project) if julia_project else _DEFAULT_JULIA_PROJECT

    if hasattr(source, "path"):
        src_path = source.path
        if src_path is None:
            raise RuntimeError("SourceFunction has no on-disk path; save it first.")
    else:
        src_path = str(source)

    if out is None:
        out = proj / "outputs" / "image.h5"
    out = Path(out).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)

    args = [
        "--source", str(Path(src_path).resolve()),
        "--incl",   f"{float(inclination_deg)}",
        "--azim",   f"{float(azimuth_deg)}",
        "--res",    str(int(res)),
        "--fov-au", f"{float(fov_au)}",
        "--nsteps", str(int(nsteps)),
        "--out",    str(out),
    ]
    if wavelength is not None:
        args += ["--wavelength", f"{float(wavelength)}"]
    if not include_thermal:
        args += ["--no-thermal"]
    if not include_scatter:
        args += ["--no-scatter"]
    if not png:
        args += ["--no-png"]
    else:
        args += ["--cmap", str(cmap), "--log-png", "true" if log_png else "false"]

    _run_julia(_IMG_SCRIPT, args, proj, verbose)

    if not load_result:
        return None
    return Image.load(out)
