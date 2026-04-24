module OpacityTable

export DustOpacity, load_opacity, interp_opacity

struct DustOpacity{T<:Real}
    lambda_um::Vector{T}      # ascending wavelength, μm
    log_lambda::Vector{T}
    kabs::Vector{T}           # cm^2/g
    ksca::Vector{T}           # cm^2/g
    g_asym::Vector{T}         # <cos θ>
end

"""
    load_opacity(path; T=Float64)

Parse the RADMC-3D-style opacity file at `path`. Format:
comment lines starting with `#`, an iformat integer, a wavelength-count integer,
then columns (λ[μm], κ_abs, κ_sca, g).
"""
function load_opacity(path::AbstractString; T::Type=Float64)
    data = Matrix{T}(undef, 0, 4)
    nlam_expected = 0
    iformat = 0
    numbers_seen = 0
    rows = Vector{NTuple{4,T}}()

    for raw in eachline(path)
        line = strip(raw)
        isempty(line) && continue
        startswith(line, "#") && continue

        tokens = split(line)
        if numbers_seen == 0 && length(tokens) == 1
            iformat = parse(Int, tokens[1])
            numbers_seen = 1
            continue
        elseif numbers_seen == 1 && length(tokens) == 1
            nlam_expected = parse(Int, tokens[1])
            numbers_seen = 2
            continue
        end

        if length(tokens) >= 4
            λ   = parse(T, tokens[1])
            ka  = parse(T, tokens[2])
            ks  = parse(T, tokens[3])
            gs  = parse(T, tokens[4])
            push!(rows, (λ, ka, ks, gs))
        end
    end

    nlam = length(rows)
    nlam_expected != 0 && nlam != nlam_expected &&
        @warn "opacity file header declared nlam=$nlam_expected but parsed $nlam rows"

    λ = [r[1] for r in rows]
    perm = sortperm(λ)
    λ  = λ[perm]
    ka = [r[2] for r in rows][perm]
    ks = [r[3] for r in rows][perm]
    gs = [r[4] for r in rows][perm]

    return DustOpacity{T}(λ, log.(λ), ka, ks, gs)
end

"""
    interp_opacity(tab, λ_um) -> (kabs, ksca, g)

Log-log interpolation of κ, linear interpolation of g, at wavelength λ_um (μm).
Clamps to table endpoints.
"""
function interp_opacity(tab::DustOpacity{T}, λ_um::Real) where {T}
    x = T(log(λ_um))
    lx = tab.log_lambda
    n = length(lx)
    if x <= lx[1]
        return tab.kabs[1], tab.ksca[1], tab.g_asym[1]
    elseif x >= lx[end]
        return tab.kabs[end], tab.ksca[end], tab.g_asym[end]
    end

    lo, hi = 1, n
    while hi - lo > 1
        m = (lo + hi) >> 1
        if lx[m] <= x
            lo = m
        else
            hi = m
        end
    end
    t = (x - lx[lo]) / (lx[hi] - lx[lo])

    log_ka = (1-t) * log(tab.kabs[lo]) + t * log(tab.kabs[hi])
    log_ks = (1-t) * log(tab.ksca[lo]) + t * log(tab.ksca[hi])
    g_val  = (1-t) * tab.g_asym[lo]    + t * tab.g_asym[hi]
    return exp(log_ka), exp(log_ks), g_val
end

end # module
