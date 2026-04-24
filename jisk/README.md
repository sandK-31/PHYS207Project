# jisk

GPU-accelerated radiative transfer for a 3D protoplanetary disk, in Julia + CUDA.jl.

Two modules:

- **Module 1 — Monte Carlo engine** (`src/MonteCarlo.jl`). Simulates photon
  packets emitted from a central source, tracking positions and directions in
  Cartesian internally and indexing a cylindrical `(r, φ, z)` grid on the fly.
  Uses Woodcock (null-collision) tracking with atomic accumulation into a 3D
  scattering source function grid. Supports isotropic and Henyey–Greenstein
  phase functions.
- **Module 2 — Formal ray-tracer** (`src/RayTrace.jl`). Takes the converged
  cylindrical source function, density, and opacities, and integrates the
  radiative transfer equation along per-pixel rays at an arbitrary observer
  inclination and azimuth. Returns a 2D intensity image. Includes scattered and
  thermal (Planck) emission.

All physical inputs (disk geometry, opacity, etc.) are read from the shared repo
data in `../data/`.

---

## Physical setup

- **Dust density**: Chiang & Goldreich (1997) flaring-disk model, described in
  [../data/dust_profile.md](../data/dust_profile.md).
- **Opacity**: amorphous olivine, [../data/opacity.dat](../data/opacity.dat),
  interpolated at the requested wavelength. A single wavelength per MC run —
  `(κ_abs, κ_sca, g)` passed as scalars.
- **Grid**: cylindrical `(r, φ, z)` with log-spaced `r`, linear `φ`, linear `z`.
  Default resolution `128 × 64 × 128`.

---

## Quick start

```bash
cd jisk
julia --project=. -e 'using Pkg; Pkg.instantiate()'

# 1) Monte Carlo: produce 3D scattering source function at λ = 1 μm
julia --project=. scripts/run_mc.jl --wavelength 1.0 --npackets 1e7

# 2) Ray-trace the saved source function at inclination 60°, 512×512
julia --project=. scripts/make_image.jl \
    --source outputs/S_sca.h5 --incl 60 --azim 0 --res 512
```

Outputs land in `outputs/`:
- `S_sca.h5` — scattering source function (erg s⁻¹ cm⁻² sr⁻¹) on the cylindrical grid
- `S_sca_Eabs.h5` — per-voxel absorbed photon weight (multiply by `L★/N_p` for erg/s)
- `image.h5` — 2D intensity image (erg s⁻¹ cm⁻² sr⁻¹)
- `image.png` — colormapped sibling PNG (log10 by default; `--no-png` to skip)

Re-image at any observer angle without re-running the MC.

### Driver flags

`run_mc.jl`:
- `--wavelength <μm>`, `--npackets <N>`, `--phase {iso|hg}`
- `--Nr <N>`, `--Nphi <N>`, `--Nz <N>` — grid resolution
- `--seed <uint>`, `--max-scatter <N>`
- `--out <path>` — output HDF5 path

`make_image.jl`:
- `--source <path>` — required; the saved source function
- `--incl <deg>`, `--azim <deg>`
- `--res <N>`, `--fov-au <AU>`, `--nsteps <N>`
- `--wavelength <μm>` — override wavelength for imaging (e.g., thermal at 10 μm from scatter computed at 1 μm; mixes modes — use only when you know what you want)
- `--no-thermal`, `--no-scatter` — disable either emission channel
- `--no-png` — skip the sibling PNG render (HDF5 is always written)
- `--cmap <name>` — any ColorSchemes name, e.g. `inferno` (default), `viridis`, `magma`
- `--log-png true|false` — log10 stretch for the PNG (default true)
- `--out <path>` — `.h5` output; PNG is saved at the same stem with `.png`

---

## Python interface

A radmc3dPy-style Python wrapper lives in [`python/`](python/README.md). It
subprocess-drives the Julia entry points and loads HDF5 outputs back as numpy
arrays.

```python
import jisk as qd
src = qd.run_mc(wavelength=1.0, npackets=int(1e7))
img = src.render(inclination_deg=60, res=512, fov_au=250)
qd.plot_image(img)
```

Install with `pip install -e jisk/python` (see [python/README.md](python/README.md)).

## Module layout

```
jisk/
├── Project.toml
├── README.md
├── src/
│   ├── Jisk.jl        # top-level module
│   ├── Constants.jl        # cgs constants, Planck B_ν
│   ├── Grid.jl             # cylindrical grid + device-compatible indexing
│   ├── DiskProfile.jl      # Chiang & Goldreich ρ(r,z), T_mid(r), h(r)
│   ├── OpacityTable.jl     # reader + (κ_abs, κ_sca, g)(λ) interpolation
│   ├── Phase.jl            # isotropic + HG sampling, frame rotation
│   ├── MonteCarlo.jl       # Module 1 CUDA kernel (Woodcock, atomics)
│   ├── RayTrace.jl         # Module 2 CUDA kernel (per-pixel formal integration)
│   └── IO.jl               # HDF5 save/load helpers
├── test/
│   ├── runtests.jl
│   ├── test_grid.jl
│   ├── test_phase.jl
│   ├── test_opacity.jl
│   ├── test_disk.jl
│   ├── test_mc_homogeneous.jl
│   ├── test_raytrace.jl
│   └── test_energy.jl
└── scripts/
    ├── run_mc.jl
    └── make_image.jl
```

---

## Architectural notes

**Coordinate strategy.** Photons propagate in Cartesian — `r += Δs · d` is
straight-line and branch-free. Only when an event needs a grid index is the
position converted: `r_cyl = hypot(x,y)`, `φ = atan(y,x)`, with a branchless
log-radial binary search for the radial bin (`Grid.index_r`). The index helpers
(`Grid.cyl_indices_from_cart`) are `@inline` and device-compatible.

**Monte Carlo propagation.** Woodcock (null-collision / delta) tracking is used
throughout Module 1. Each step samples a free path `Δs = −log(ξ)/ρκ_max` using
the global majorant, advances the photon in Cartesian, and with probability
`ρ/ρ_max` commits to a real collision (absorption vs scattering decided next by
`κ_abs/κ_ext`). No voxel-boundary intersections are computed during the MC —
this keeps the kernel branch-light and GPU-friendly. Atomic `CUDA.@atomic` adds
accumulate packet weights into `S_accum[ir, iφ, iz]` on scatter events and into
`E_abs[ir, iφ, iz]` on absorption.

**Source-function normalization.** Given `N_p` packets each carrying `L★/N_p`,
```
S_sca[i,j,k] = Σ_deposits · (L★ / N_p) / (4π · ρ[i,j,k] · κ_sca · V_voxel[i,j,k])
```
Voxels with ρ = 0 are set to 0. See `MonteCarlo.source_function_from_accum`.

**Formal ray-tracer.** Module 2 is a per-pixel kernel. Each thread:
1. Computes its ray origin on the observer-side face of the bounding cylinder
   and a direction `−d_obs`.
2. Finds the ray’s `t_enter`, `t_exit` through the finite cylinder.
3. Marches `n_steps` sample points along the ray using fixed `ds`; at each
   point it reads `ρ`, `S_sca`, and `T_mid(r)` (for `B_ν`), builds
   `j = ρκ_sca S_sca + ρκ_abs B_ν`, and accumulates
   `I += S · (1 − e^{−Δτ}) · e^{−τ}` while advancing `τ`.
4. Stops when `τ > 30` (saturation) or the sample exits the grid.

A fixed-step march is used rather than exact cylindrical 3D-DDA for robustness
— no corner-case handling needed at voxel boundaries, and the march spacing is
chosen to be small relative to the grid (`n_steps ≥ 3 · max(N_r, N_z)` is
recommended).

**GPU correctness.** `CUDA.allowscalar(false)` is set in `run_mc!` and
`render_image`. All accumulation uses `CUDA.@atomic` on `Float32` grids. The
per-thread RNG is an inlined xorshift64★ seeded via SplitMix64 from
`(base_seed, thread_id)` — independent, reproducible streams with zero kernel
allocations.

---

## Testing

```bash
julia --project=. test/runtests.jl
```

Covers:

1. **Grid** — index round-trip, voxel-volume closure (`Σ V = π(r_max²−r_min²)·2z_max`).
2. **Phase** — isotropic statistics (`⟨μ⟩ ≈ 0`, `⟨μ²⟩ ≈ 1/3`), HG `⟨μ⟩ ≈ g`, unit-norm preservation.
3. **Opacity** — interpolation reproduces table nodes exactly; log-log monotonicity; endpoint clamping.
4. **Disk profile** — `Σ(AU) = Σ_{d0}`; mass integral matches `2π Σ_{d0} AU (R_out − R_in)` within a few percent on a 128×256 grid.
5. **MC homogeneous** (GPU) — constant-density slab, energy accounting: `n_abs + n_esc + n_rr = N_p`.
6. **Ray-tracer sanity** (GPU) — empty disk → zero image; uniform source → nonzero bounded image; thermal-only path.
7. **Energy conservation** (GPU) — every packet terminates exactly once; all deposits nonnegative.

GPU tests are skipped when `CUDA.functional()` is false.

---

## Reproducing a reference run

```bash
julia --project=. scripts/run_mc.jl --wavelength 1.0 --npackets 10000000 --seed 0xC0FFEE
julia --project=. scripts/make_image.jl --source outputs/S_sca.h5 --incl 60 --azim 0 --res 512 --fov-au 250
```

Sanity check: rerun with `--no-scatter --no-thermal` disabled independently to
isolate channels. A `--no-scatter` + `include_thermal` image at long wavelength
(e.g., 100 μm) should match an optically-thin `∫ ρ κ_abs B_ν ds` through the disk.

---

## Notes and caveats

- **Single wavelength per MC run.** Multi-wavelength studies = repeated driver invocations. Source-function files are self-describing via their HDF5 metadata.
- **Central source model.** The star is treated as a point source at the origin with total luminosity `L★`. For an extended photosphere, modify the emission sampler in `_mc_kernel!`.
- **Axisymmetric density.** The grid is 3D but the Chiang & Goldreich density only depends on `(r, z)`. The φ dimension is reserved for non-axisymmetric source fields and future extensions (e.g., spiral structure, localized heating sources).
- **No direct-star term in images yet.** Scattered light is integrated, but the unscattered stellar flux transmitted through the disk on the line of sight is not currently added; easy to extend in `RayTrace.jl` if needed.
