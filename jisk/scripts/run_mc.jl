#!/usr/bin/env julia
# Run Module 1: Monte Carlo photon transport → 3D scattering source function.
#
# Usage:
#   julia --project=. scripts/run_mc.jl \
#       [--wavelength 1.0] [--npackets 1e7] [--phase hg] \
#       [--Nr 128] [--Nphi 64] [--Nz 128] \
#       [--seed 12345] [--max-scatter 100] \
#       [--out outputs/S_sca.h5]

using Printf

push!(LOAD_PATH, joinpath(@__DIR__, "..", "src"))
include(joinpath(@__DIR__, "..", "src", "Jisk.jl"))
using .Jisk
using .Jisk.Constants
using CUDA

function parse_args(args)
    opts = Dict{String,Any}(
        "wavelength"    => 1.0,
        "npackets"      => 1_000_000,
        "phase"         => "hg",
        "g"             => nothing,        # nothing → use opacity-table value
        "r-in-au"       => Float64(Constants.R_in_AU),
        "r-out-au"      => Float64(Constants.R_out_AU),
        "alpha"         => Float64(Constants.alpha_graze),
        "z-max-au"      => nothing,        # nothing → 5 × h(R_out)
        "Nr"            => 128,
        "Nphi"          => 64,
        "Nz"            => 128,
        "seed"          => 0xC0FFEE_BEEF_CAFE,
        "max-scatter"   => 100,
        "out"           => joinpath(@__DIR__, "..", "outputs", "S_sca.h5"),
    )
    i = 1
    while i <= length(args)
        k = args[i]
        startswith(k, "--") || error("unexpected arg: $k")
        key = k[3:end]
        v   = args[i+1]
        opts[key] = v
        i += 2
    end
    opts["wavelength"]   = parse(Float64, string(opts["wavelength"]))
    opts["npackets"]     = Int64(parse(Float64, string(opts["npackets"])))
    opts["Nr"]           = parse(Int,   string(opts["Nr"]))
    opts["Nphi"]         = parse(Int,   string(opts["Nphi"]))
    opts["Nz"]           = parse(Int,   string(opts["Nz"]))
    opts["seed"]         = UInt64(parse(UInt, string(opts["seed"]); base = startswith(string(opts["seed"]), "0x") ? 16 : 10))
    opts["max-scatter"]  = parse(Int,   string(opts["max-scatter"]))
    opts["r-in-au"]      = parse(Float64, string(opts["r-in-au"]))
    opts["r-out-au"]     = parse(Float64, string(opts["r-out-au"]))
    opts["alpha"]        = parse(Float64, string(opts["alpha"]))
    if opts["g"] !== nothing
        opts["g"] = parse(Float64, string(opts["g"]))
    end
    if opts["z-max-au"] !== nothing
        opts["z-max-au"] = parse(Float64, string(opts["z-max-au"]))
    end
    return opts
end

function main()
    opts = parse_args(ARGS)
    @info "jisk MC run" opts=opts

    # Grid (Float32) --------------------------------------------------
    # z_max defaults to 5 × scale-height at R_out (with the chosen alpha)
    z_max_au = if opts["z-max-au"] === nothing
        5.0 * (Jisk.DiskProfile.scale_height(opts["r-out-au"] * Constants.AU;
                                             alpha=opts["alpha"]) / Constants.AU)
    else
        opts["z-max-au"]
    end
    grid = build_grid(; N_r=opts["Nr"], N_phi=opts["Nphi"], N_z=opts["Nz"],
                       r_min_AU=opts["r-in-au"], r_max_AU=opts["r-out-au"],
                       z_max_AU=z_max_au, T=Float32)

    # Density (on host, then upload) -----------------------------------
    ρ_host = build_density_array(grid;
                                 r_in_au=opts["r-in-au"],
                                 r_out_au=opts["r-out-au"],
                                 alpha=opts["alpha"])
    ρ_d = CuArray(ρ_host)

    # Opacity at requested wavelength ----------------------------------
    opac_path = joinpath(@__DIR__, "..", "..", "data", "opacity.dat")
    tab = load_opacity(opac_path; T=Float64)
    κ_abs, κ_sca, g_table = interp_opacity(tab, opts["wavelength"])
    g_asym = opts["g"] === nothing ? g_table : opts["g"]
    @info "opacities" κ_abs κ_sca g_asym g_table_default=g_table

    # Phase
    phase = Symbol(opts["phase"])

    # Run Monte Carlo --------------------------------------------------
    t0 = time()
    res = run_mc!(grid, ρ_d;
                  κ_abs=κ_abs, κ_sca=κ_sca, g=g_asym, phase=phase,
                  N_p=opts["npackets"], max_scatter=opts["max-scatter"],
                  seed=opts["seed"])
    dt = time() - t0

    n_abs = Array(res.n_absorbed)[1]
    n_esc = Array(res.n_escaped)[1]
    n_rr  = Array(res.n_rr_killed)[1]
    @info "MC summary" duration_s=dt n_absorbed=n_abs n_escaped=n_esc n_rr=n_rr

    # Build physical S_sca --------------------------------------------
    V_host = voxel_volume_array(grid)   # Float64, cm^3
    S_sca_h = Jisk.MonteCarlo.source_function_from_accum(
        res.S_accum, ρ_host, V_host, κ_sca, Constants.L_star, opts["npackets"])

    # Save ----------------------------------------------------------
    out = opts["out"]
    mkpath(dirname(out))
    metadata = Dict(
        "wavelength_um" => opts["wavelength"],
        "N_packets"     => Int(opts["npackets"]),
        "seed"          => UInt64(opts["seed"]),
        "kappa_abs"     => Float64(κ_abs),
        "kappa_sca"     => Float64(κ_sca),
        "g_asym"        => Float64(g_asym),
        "phase"         => String(opts["phase"]),
        "r_in_au"       => Float64(opts["r-in-au"]),
        "r_out_au"      => Float64(opts["r-out-au"]),
        "alpha_graze"   => Float64(opts["alpha"]),
        "z_max_au"      => Float64(z_max_au),
        "runtime_s"     => dt,
        "n_absorbed"    => Int(n_abs),
        "n_escaped"     => Int(n_esc),
        "n_rr"          => Int(n_rr),
    )
    save_source_function(out, S_sca_h, grid; metadata=metadata)
    save_absorbed_energy(replace(out, ".h5" => "_Eabs.h5"), Array(res.E_abs), grid; metadata=metadata)

    @info "saved" source_function=out abs_energy=replace(out, ".h5" => "_Eabs.h5")
    return 0
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
