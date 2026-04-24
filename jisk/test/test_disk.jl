@testset "Disk profile" begin
    # T_mid at 1 AU from analytic: F_irr = α L★ / (4π (AU)²); T = (F/σ)^(1/4)
    F_irr_1AU = Constants.alpha_graze * Constants.L_star / (4π * Constants.AU^2)
    T_1AU = (F_irr_1AU / Constants.sigma_SB)^(1/4)
    @test isapprox(midplane_temperature(Constants.AU), T_1AU; rtol=1e-8)

    # Σ_d(AU) = Σ_{d0}
    @test isapprox(surface_density(Constants.AU), Constants.Sigma_d0; rtol=1e-12)

    # Scale height ratio h/r should be small (O(0.01)) for a typical AU
    r = 50 * Constants.AU
    h = scale_height(r)
    @test 0 < h / r < 0.2

    # Mass integral: ∫ ρ dV over the 3D grid ≈ ∫ Σ(r) dA (column integral)
    # because the vertical Gaussian is normalized. Use a fine grid and a vertical
    # domain wide enough (≥ 5 scale heights at R_out).
    grid = build_grid(; N_r=128, N_phi=1, N_z=256,
                       r_min_AU=10.0, r_max_AU=100.0,
                       z_max_AU=Float64((scale_height(100 * Constants.AU) / Constants.AU) * 6),
                       T=Float64)
    ρ = build_density_array(grid)
    V = voxel_volume_array(grid)
    M_dust = sum(ρ .* V)

    # Analytic column: M = ∫_{R_in}^{R_out} Σ(r) 2π r dr = 2π Σ_{d0} AU · (R_out - R_in)
    # for p_sig = -1
    M_analytic = 2π * Constants.Sigma_d0 * Constants.AU *
                 (Constants.R_out_AU - Constants.R_in_AU) * Constants.AU
    @test isapprox(M_dust, M_analytic; rtol=5e-2)  # <5% on this grid

    # Non-negativity
    @test all(ρ .>= 0)

    # Density peaks at midplane (z=0)
    ρ_mid = ρ[64, 1, 128]   # midplane-ish
    ρ_off = ρ[64, 1, 200]   # off-midplane
    @test ρ_mid > ρ_off
end
