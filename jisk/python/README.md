# jisk — Python interface

A thin Python wrapper around the Julia/CUDA.jl `jisk` radiative-transfer
engine, in the style of `radmc3dPy`. It subprocess-drives the Julia
`scripts/run_mc.jl` and `scripts/make_image.jl` entry points and reads the
HDF5 outputs back as numpy arrays.

## Install

From the repo root:

```bash
pip install -e jisk/python
# optional plotting extras:
pip install -e "jisk/python[plot]"     # matplotlib (2D)
pip install -e "jisk/python[viz3d]"    # plotly (interactive 3D)
pip install -e "jisk/python[all]"      # both
```

You also need:
- Julia ≥ 1.10 on `$PATH` (or point at one with `set_julia_binary()` / `$JISK_JULIA`)
- `jisk/Project.toml` already instantiated:
  `cd jisk && julia --project=. -e 'using Pkg; Pkg.instantiate()'`
- A CUDA GPU (CUDA.jl must be functional)

## Quick start

```python
import jisk as qd

# 1) Monte Carlo: build the scattering source function at λ = 1 μm
src = qd.run_mc(wavelength=1.0, npackets=int(1e7),
                Nr=128, Nphi=64, Nz=128, phase="hg", seed=0xC0FFEE)

print(src.S_sca.shape)           # (128, 64, 128)
print(src.wavelength_um)         # 1.0
print(src.metadata["n_absorbed"], src.metadata["n_escaped"])

# 2) Ray-trace at an arbitrary observer orientation (no MC rerun)
img = src.render(inclination_deg=60, azimuth_deg=0, res=512, fov_au=250)

print(img.image.shape)           # (512, 512)
qd.plot_image(img)               # requires matplotlib
```

Or drive the ray-tracer on an existing file:

```python
img = qd.render_image(
    source="jisk/outputs/S_sca.h5",
    inclination_deg=30, azimuth_deg=45, res=1024, fov_au=300,
    include_thermal=True, include_scatter=True,
    png=True, cmap="inferno", log_png=True,      # sibling PNG controls
)
print(img.png_path)      # path to the auto-written PNG, or None if disabled
```

Other viewing angles are cheap — `run_mc` builds the 3D source function once,
then each `render_image` call is just ray-tracing.

## API

```python
qd.run_mc(*, wavelength, npackets, phase, Nr, Nphi, Nz, seed, max_scatter, out, ...) -> SourceFunction
qd.render_image(*, source, inclination_deg, azimuth_deg, res, fov_au, nsteps,
                wavelength, include_thermal, include_scatter, out, ...) -> Image

qd.SourceFunction.load("S_sca.h5")        # loads any file written by run_mc
qd.AbsorbedEnergy.load("S_sca_Eabs.h5")
qd.Image.load("image.h5")

# Plain analytic disk profile (mirrors DiskProfile.jl, NumPy-vectorized)
qd.midplane_temperature(r_au=50)          # → K
qd.surface_density(r_au=50)               # → g/cm^2
qd.scale_height(r_au=50)                  # → cm
qd.density_cyl(r_au=[10, 50], z_au=[0, 0.1])  # → g/cm^3 (broadcasted)

qd.plot_image(img, log=True)              # 2D sky image (matplotlib)
qd.plot_source_slice(src, phi_index=0)    # (r, z) slice of S_sca (matplotlib)
qd.plot_3d(src, out="source_3d.html")     # interactive 3D (plotly, opens in browser)
```

## Notes

- All units are cgs on the Python side. `r_au` / `z_au` are provided as convenience but `r_cm` / `z_cm` also work.
- `Image.metadata` and `SourceFunction.metadata` carry the full HDF5 attribute dict from the Julia driver — wavelength, κ_abs/κ_sca/g, N_packets, RNG seed, runtime, etc.
- `run_mc` writes two HDF5 files: `<out>.h5` (source function) and `<out>_Eabs.h5` (absorbed energy). Load the latter with `qd.AbsorbedEnergy.load(...)`.
- If the Julia subprocess fails, the stderr is printed and a `RuntimeError` is raised.
