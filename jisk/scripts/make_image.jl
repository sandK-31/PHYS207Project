#!/usr/bin/env julia
# Run Module 2: formal ray-tracer on a saved scattering source function.
#
# Usage:
#   julia --project=. scripts/make_image.jl \
#       --source outputs/S_sca.h5 \
#       [--incl 60] [--azim 0] [--res 512] [--fov-au 250] [--nsteps 512] \
#       [--wavelength 1.0] \
#       [--no-thermal] [--no-scatter] \
#       [--out outputs/image.h5]

using Printf
using HDF5

push!(LOAD_PATH, joinpath(@__DIR__, "..", "src"))
include(joinpath(@__DIR__, "..", "src", "Jisk.jl"))
using .Jisk
using .Jisk.Constants
using CUDA

function parse_args(args)
    opts = Dict{String,Any}(
        "source"      => nothing,
        "incl"        => 60.0,
        "azim"        => 0.0,
        "res"         => 512,
        "fov-au"      => 250.0,
        "nsteps"      => 512,
        "wavelength"  => nothing,
        "no-thermal"  => false,
        "no-scatter"  => false,
        "no-png"      => false,
        "cmap"        => "inferno",
        "log-png"     => true,
        "out"         => joinpath(@__DIR__, "..", "outputs", "image.h5"),
    )
    i = 1
    while i <= length(args)
        k = args[i]
        startswith(k, "--") || error("unexpected arg: $k")
        key = k[3:end]
        if key in ("no-thermal", "no-scatter", "no-png")
            opts[key] = true
            i += 1
        elseif key == "log-png"
            opts[key] = parse(Bool, args[i+1])
            i += 2
        else
            opts[key] = args[i+1]
            i += 2
        end
    end
    opts["source"] !== nothing || error("--source <path> is required")
    opts["incl"]       = parse(Float64, string(opts["incl"]))
    opts["azim"]       = parse(Float64, string(opts["azim"]))
    opts["res"]        = parse(Int,     string(opts["res"]))
    opts["fov-au"]     = parse(Float64, string(opts["fov-au"]))
    opts["nsteps"]     = parse(Int,     string(opts["nsteps"]))
    if opts["wavelength"] !== nothing
        opts["wavelength"] = parse(Float64, string(opts["wavelength"]))
    end
    return opts
end

function main()
    opts = parse_args(ARGS)
    @info "jisk ray-trace" opts

    S_sca_host, g_edges = load_source_function(opts["source"])

    # Recover grid dims from edges
    N_r   = length(g_edges.r_edges) - 1
    N_phi = length(g_edges.phi_edges) - 1
    N_z   = length(g_edges.z_edges) - 1

    # Load metadata (wavelength, κ, disk params) from the source file.
    # Disk params (R_in, R_out, alpha) fall back to library defaults when absent
    # — keeps backward compatibility with older source-function files.
    meta = h5open(opts["source"], "r") do f
        mg = f["metadata"]
        d = Dict{String,Any}(
            "wavelength_um" => read_attribute(mg, "wavelength_um"),
            "kappa_abs"     => read_attribute(mg, "kappa_abs"),
            "kappa_sca"     => read_attribute(mg, "kappa_sca"),
        )
        for (k, default) in (("r_in_au",     Float64(Constants.R_in_AU)),
                             ("r_out_au",    Float64(Constants.R_out_AU)),
                             ("alpha_graze", Float64(Constants.alpha_graze)))
            d[k] = haskey(HDF5.attributes(mg), k) ? read_attribute(mg, k) : default
        end
        d
    end

    λ_um = opts["wavelength"] === nothing ? meta["wavelength_um"] : opts["wavelength"]
    κ_abs = meta["kappa_abs"]
    κ_sca = meta["kappa_sca"]
    if opts["wavelength"] !== nothing
        # Re-interpolate opacity at the (possibly different) imaging wavelength
        opac_path = joinpath(@__DIR__, "..", "..", "data", "opacity.dat")
        tab = load_opacity(opac_path; T=Float64)
        κ_abs, κ_sca, _ = interp_opacity(tab, λ_um)
    end
    r_in_au  = Float64(meta["r_in_au"])
    r_out_au = Float64(meta["r_out_au"])
    alpha    = Float64(meta["alpha_graze"])
    @info "imaging" wavelength_um=λ_um κ_abs κ_sca r_in_au r_out_au alpha

    # Rebuild grid object from loaded edges
    r_min_AU = Float64(g_edges.r_edges[1])   / Constants.AU
    r_max_AU = Float64(g_edges.r_edges[end]) / Constants.AU
    z_max_AU = Float64(g_edges.z_edges[end]) / Constants.AU
    grid = build_grid(; N_r=N_r, N_phi=N_phi, N_z=N_z,
                       r_min_AU=r_min_AU, r_max_AU=r_max_AU,
                       z_max_AU=z_max_AU, T=Float32)

    ρ_host = build_density_array(grid; r_in_au=r_in_au, r_out_au=r_out_au, alpha=alpha)
    T_mid  = build_T_mid_array(grid; alpha=alpha)

    ρ_d  = CuArray(ρ_host)
    S_d  = CuArray(Float32.(S_sca_host))

    obs = ObserverParams(;
        inclination = Float32(deg2rad(opts["incl"])),
        azimuth     = Float32(deg2rad(opts["azim"])),
        N_px        = Int32(opts["res"]),
        N_py        = Int32(opts["res"]),
        fov_cm      = Float32(opts["fov-au"] * Constants.AU),
        n_steps     = Int32(opts["nsteps"]),
    )

    t0 = time()
    img = render_image(grid, ρ_d, S_d, obs;
                       κ_abs=κ_abs, κ_sca=κ_sca, λ_um=λ_um,
                       T_mid_r=T_mid,
                       include_thermal=!opts["no-thermal"],
                       include_scatter=!opts["no-scatter"])
    dt = time() - t0
    @info "render complete" duration_s=dt

    out = opts["out"]
    mkpath(dirname(out))
    meta_out = Dict(
        "wavelength_um"      => λ_um,
        "kappa_abs"          => Float64(κ_abs),
        "kappa_sca"          => Float64(κ_sca),
        "include_thermal"    => !opts["no-thermal"],
        "include_scatter"    => !opts["no-scatter"],
        "source_file"        => opts["source"],
        "runtime_s"          => dt,
    )
    img_host = Array(img)
    save_image(out, img_host, obs; metadata=meta_out)
    @info "saved image" path=out

    if !opts["no-png"]
        png_path = replace(out, ".h5" => ".png")
        Jisk.IO.save_image_png(png_path, img_host;
            log = opts["log-png"], cmap = Symbol(opts["cmap"]))
        @info "saved png" path=png_path
    end
    return 0
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
