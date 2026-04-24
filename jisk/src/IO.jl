module IO

using HDF5
using CUDA
using ColorSchemes
using PNGFiles
using Statistics
using ..Grid

export save_source_function, save_absorbed_energy, save_image, save_image_png,
       load_source_function

_to_host(a::CuArray) = Array(a)
_to_host(a::AbstractArray) = a

function _write_grid_attrs(g, grid::CylGrid)
    g["r_edges"]   = Array(grid.r_edges)
    g["phi_edges"] = Array(grid.phi_edges)
    g["z_edges"]   = Array(grid.z_edges)
    HDF5.attributes(g)["N_r"]   = Int(grid.N_r)
    HDF5.attributes(g)["N_phi"] = Int(grid.N_phi)
    HDF5.attributes(g)["N_z"]   = Int(grid.N_z)
    HDF5.attributes(g)["r_min_cm"] = Float64(grid.r_min)
    HDF5.attributes(g)["r_max_cm"] = Float64(grid.r_max)
    HDF5.attributes(g)["z_min_cm"] = Float64(grid.z_min)
    HDF5.attributes(g)["z_max_cm"] = Float64(grid.z_max)
end

"""
    save_source_function(path, S_sca, grid; metadata=Dict())

Writes an HDF5 file with the 3D scattering source function (N_r, N_phi, N_z) plus
the cylindrical grid edges and arbitrary metadata (e.g., wavelength, N_packets, seed).
"""
function save_source_function(path::AbstractString, S_sca::AbstractArray{<:Real,3},
                              grid::CylGrid; metadata::AbstractDict=Dict{String,Any}())
    h5open(path, "w") do f
        f["S_sca"] = _to_host(S_sca)
        g = create_group(f, "grid")
        _write_grid_attrs(g, grid)
        meta = create_group(f, "metadata")
        for (k, v) in metadata
            HDF5.attributes(meta)[String(k)] = v
        end
        HDF5.attributes(f)["units_S_sca"] = "erg s^-1 cm^-2 sr^-1"
    end
    return path
end

"""
    save_absorbed_energy(path, E_abs, grid; metadata=Dict())

Writes the 3D absorbed-energy grid (per-voxel packet-weight sums; convert to
erg s^-1 by multiplying by L★/N_p).
"""
function save_absorbed_energy(path::AbstractString, E_abs::AbstractArray{<:Real,3},
                              grid::CylGrid; metadata::AbstractDict=Dict{String,Any}())
    h5open(path, "w") do f
        f["E_abs"] = _to_host(E_abs)
        g = create_group(f, "grid")
        _write_grid_attrs(g, grid)
        meta = create_group(f, "metadata")
        for (k, v) in metadata
            HDF5.attributes(meta)[String(k)] = v
        end
        HDF5.attributes(f)["units_E_abs"] = "packet-weight per voxel (multiply by L_star/N_p for erg/s)"
    end
    return path
end

"""
    save_image(path, img, obs_params; metadata=Dict())

Writes a 2D image (N_px, N_py) of specific intensity plus observer metadata.
"""
function save_image(path::AbstractString, img::AbstractArray{<:Real,2},
                    obs_params; metadata::AbstractDict=Dict{String,Any}())
    h5open(path, "w") do f
        f["image"] = _to_host(img)
        o = create_group(f, "observer")
        HDF5.attributes(o)["inclination_rad"] = Float64(obs_params.inclination)
        HDF5.attributes(o)["azimuth_rad"]     = Float64(obs_params.azimuth)
        HDF5.attributes(o)["N_px"]            = Int(obs_params.N_px)
        HDF5.attributes(o)["N_py"]            = Int(obs_params.N_py)
        HDF5.attributes(o)["fov_cm"]          = Float64(obs_params.fov_cm)
        HDF5.attributes(o)["n_steps"]         = Int(obs_params.n_steps)
        meta = create_group(f, "metadata")
        for (k, v) in metadata
            HDF5.attributes(meta)[String(k)] = v
        end
        HDF5.attributes(f)["units_image"] = "erg s^-1 cm^-2 sr^-1"
    end
    return path
end

"""
    save_image_png(path, img; log=true, cmap=:inferno,
                   pmin=0.0, pmax=100.0, floor_abs=1e-30)

Write a colormapped PNG of the 2D intensity image.

Default behavior mirrors `jisk.plot_image` in Python: log10 stretch with
an absolute floor at `floor_abs`, and the colormap spans the full data range
(`pmin=0`, `pmax=100`). This keeps faint features (e.g. the outer disk edge
at high inclination) visible.

Pass `pmin`/`pmax` (percentiles) or a larger `floor_abs` to tighten contrast
around the peak if you prefer — e.g. `pmin=1, pmax=99.5, floor_abs=maximum(img)*1e-6`.

Arguments:
- `log=true`: apply log10 stretch.
- `cmap`: any `ColorSchemes.jl` scheme, e.g. `:inferno`, `:viridis`, `:magma`.
- `floor_abs`: absolute lower clamp applied before log10 (ignored if `log=false`).
- `pmin` / `pmax`: percentile clip (0 & 100 = full data range).

Orientation: row 0 of the PNG is the top of the image, y increases upward,
x increases rightward — matching typical astronomy display conventions.
"""
function save_image_png(path::AbstractString, img::AbstractArray{<:Real,2};
                        log::Bool=true, cmap::Symbol=:inferno,
                        pmin::Real=0.0, pmax::Real=100.0,
                        floor_abs::Real=1e-30)
    data = Float64.(img)
    if log
        data = log10.(clamp.(data, Float64(floor_abs), Inf))
    end
    finite_vals = filter(isfinite, vec(data))
    if isempty(finite_vals)
        lo, hi = 0.0, 1.0
    elseif pmin <= 0 && pmax >= 100
        lo = Float64(minimum(finite_vals))
        hi = Float64(maximum(finite_vals))
    else
        lo = Float64(quantile(finite_vals, pmin/100))
        hi = Float64(quantile(finite_vals, pmax/100))
    end
    if !(hi > lo)
        hi = lo + eps(lo) + 1.0
    end
    t = clamp.((data .- lo) ./ (hi - lo), 0.0, 1.0)

    scheme = getfield(ColorSchemes, cmap)
    colored = get.(Ref(scheme), t)                        # RGB{Float64}, shape (N_px, N_py)

    # Reorient for PNG: rows = y (top-down), cols = x (left-right), y-up astro convention.
    rgb = reverse(permutedims(colored, (2, 1)), dims=1)

    PNGFiles.save(path, rgb)
    return path
end

"""
    load_source_function(path) -> (S_sca, grid_edges)

Returns the stored S_sca array (Float32, N_r × N_phi × N_z) and a NamedTuple
with the grid edges for reuse in the ray-tracer.
"""
function load_source_function(path::AbstractString)
    h5open(path, "r") do f
        S = read(f["S_sca"])
        r_edges   = read(f["grid/r_edges"])
        phi_edges = read(f["grid/phi_edges"])
        z_edges   = read(f["grid/z_edges"])
        return Float32.(S), (r_edges=r_edges, phi_edges=phi_edges, z_edges=z_edges)
    end
end

end # module
