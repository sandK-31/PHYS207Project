using Statistics
using Random

@testset "Phase functions" begin
    rng = MersenneTwister(42)

    # Isotropic: <μ> ~ 0 and <μ²> ~ 1/3
    N = 200_000
    μs_iso = [Phase.sample_isotropic(rand(rng, Float64), rand(rng, Float64))[3] for _ in 1:N]
    @test abs(mean(μs_iso))     < 0.02
    @test abs(mean(μs_iso.^2) - 1/3) < 0.02

    # Henyey-Greenstein: <μ> ~ g
    for g in (-0.5, 0.0, 0.3, 0.7)
        μs_hg = [Phase.sample_hg(g, rand(rng, Float64)) for _ in 1:N]
        @test abs(mean(μs_hg) - g) < 0.02
        @test all(-1 .<= μs_hg .<= 1)
    end

    # rotate_to_frame preserves unit norm
    for _ in 1:100
        ux, uy, uz = Phase.sample_isotropic(rand(rng), rand(rng))
        μ = 2*rand(rng) - 1
        φ = 2π * rand(rng)
        nx, ny, nz = Phase.rotate_to_frame(ux, uy, uz, μ, φ)
        @test isapprox(nx*nx + ny*ny + nz*nz, 1.0; atol=1e-6)
    end

    # rotate_to_frame with μ=1 should return the same direction (no deflection)
    for _ in 1:50
        ux, uy, uz = Phase.sample_isotropic(rand(rng), rand(rng))
        nx, ny, nz = Phase.rotate_to_frame(ux, uy, uz, 1.0, rand(rng)*2π)
        @test isapprox(nx, ux; atol=1e-4)
        @test isapprox(ny, uy; atol=1e-4)
        @test isapprox(nz, uz; atol=1e-4)
    end
end
