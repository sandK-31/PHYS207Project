@testset "Opacity interpolation" begin
    opac_path = joinpath(@__DIR__, "..", "..", "data", "opacity.dat")
    tab = load_opacity(opac_path; T=Float64)

    @test length(tab.lambda_um) > 100
    @test issorted(tab.lambda_um)
    @test all(tab.kabs .> 0)
    @test all(tab.ksca .> 0)

    # Exact interpolation at a node
    i = 100
    λ_node = tab.lambda_um[i]
    ka, ks, g = interp_opacity(tab, λ_node)
    @test isapprox(ka, tab.kabs[i]; rtol=1e-10)
    @test isapprox(ks, tab.ksca[i]; rtol=1e-10)
    @test isapprox(g,  tab.g_asym[i]; rtol=1e-10)

    # Clamps beyond endpoints
    ka_lo, _, _ = interp_opacity(tab, tab.lambda_um[1] / 10)
    @test isapprox(ka_lo, tab.kabs[1]; rtol=1e-10)
    ka_hi, _, _ = interp_opacity(tab, tab.lambda_um[end] * 10)
    @test isapprox(ka_hi, tab.kabs[end]; rtol=1e-10)

    # Monotonic/smooth between nodes: value lies between endpoint values
    λm = sqrt(tab.lambda_um[i] * tab.lambda_um[i+1])
    kam, _, _ = interp_opacity(tab, λm)
    lo, hi = minmax(tab.kabs[i], tab.kabs[i+1])
    @test lo ≤ kam ≤ hi
end
