"""
End-to-end example: set up the disk, run the Julia Monte Carlo, and plot an
image. Run from any directory — paths are relative to the script.

    python examples/basic_pipeline.py

Requires: jisk (pip install -e jisk/python), matplotlib,
Julia on $PATH, and the Julia project instantiated:
    cd jisk && julia --project=. -e 'using Pkg; Pkg.instantiate()'
"""

from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt

import jisk as qd

HERE = Path(__file__).resolve().parent
OUT  = HERE / "outputs"
OUT.mkdir(exist_ok=True)

# -- 0) Disk parameters (used both in the analytic plot and the MC) ---------
# Override the library defaults at runtime — no Julia/Python source edits needed.
disk_params = dict(
    # g           = 0.5,
    r_in_au     = 10.0,    # inner truncation radius [AU]   (default 10)
    r_out_au    = 250.0,  # outer truncation radius [AU]   (default 100)
    alpha_graze = 0.05,   # passive grazing angle α        (default 0.05)
)

# -- 1) Sanity-check the analytic disk profile ------------------------------
r_au = np.geomspace(disk_params["r_in_au"], disk_params["r_out_au"], 200)
T    = qd.midplane_temperature(r_au=r_au, alpha=disk_params["alpha_graze"])
h_au = qd.scale_height(r_au=r_au, alpha=disk_params["alpha_graze"]) / qd.AU_CM
Σ    = qd.surface_density(r_au=r_au)

fig, axs = plt.subplots(1, 3, figsize=(12, 3.5))
axs[0].loglog(r_au, T);           axs[0].set(xlabel="r [AU]", ylabel="T_mid [K]",   title="Midplane temperature")
axs[1].loglog(r_au, Σ);           axs[1].set(xlabel="r [AU]", ylabel="Σ_d [g/cm²]", title="Surface density")
axs[2].semilogx(r_au, h_au/r_au); axs[2].set(xlabel="r [AU]", ylabel="h / r",       title="Scale height ratio")
for ax in axs: ax.grid(alpha=0.3)
fig.suptitle(f"Disk profile (R_in={disk_params['r_in_au']} AU, "
             f"R_out={disk_params['r_out_au']} AU, α={disk_params['alpha_graze']})")
fig.tight_layout()
fig.savefig(OUT / "disk_profile.png", dpi=130)
print(f"saved {OUT / 'disk_profile.png'}")

# -- 2) Run the Monte Carlo at λ = 1 μm -------------------------------------
src = qd.run_mc(
    wavelength  = 1.0,
    npackets    = int(1e9),
    Nr = 256, Nphi = 256, Nz = 96,
    phase       = "hg",
    seed        = 0xC0FFEE,
    max_scatter = 1000,
    out         = OUT / "S_sca.h5",
    **disk_params,                  # forwards r_in_au, r_out_au, alpha_graze
)

print(f"S_sca shape = {src.S_sca.shape}")
print(f"absorbed = {src.metadata['n_absorbed']}, "
      f"escaped = {src.metadata['n_escaped']}, "
      f"scatters in grid = {int(src.S_sca.sum())}")

# -- 3) Plot a (r, z) slice of the source function --------------------------
fig, ax = plt.subplots(figsize=(7, 4))
qd.plot_source_slice(src, log=True, ax=ax)   # φ-averaged by default (axisymmetric disk)
fig.tight_layout()
fig.savefig(OUT / "source_rz_slice.png", dpi=130)
print(f"saved {OUT / 'source_rz_slice.png'}")

# -- 4) Render the disk at three inclinations -------------------------------
angles_deg = [0, 15, 30, 45, 60, 75, 90]
images = [src.render(inclination_deg=i, azimuth_deg=0,
                     res=1024, fov_au=800, nsteps=1024,
                     out=OUT / f"image_i{i:02d}.h5")
          for i in angles_deg]

fig, axs = plt.subplots(1, len(angles_deg), figsize=(4.7*len(angles_deg), 4.4))
for ax, img, i in zip(axs, images, angles_deg):
    qd.plot_image(img, log=True, ax=ax, title=f"i = {i}°", vmin=-6)
fig.tight_layout()
fig.savefig(OUT / "images_inclination_sweep.png", dpi=130)
print(f"saved {OUT / 'images_inclination_sweep.png'}")

# Each render also wrote a sibling PNG from the Julia driver:
for img in images:
    print(f"Julia-side PNG: {img.png_path}")

# -- 5) Interactive 3D visualization of the source function -----------------
# Writes a self-contained HTML; open in any browser to rotate/zoom/pan.
# Requires plotly:  pip install -e "jisk/python[viz3d]"
qd.plot_3d(src,
           log=True,
           N_cart=128,
           cmap="Inferno",
           title="S_sca(r, φ, z) — scattering source function",
           out=OUT / "source_3d.html",
           show=False)
print(f"saved {OUT / 'source_3d.html'}  (open in a browser)")

print("done.")
