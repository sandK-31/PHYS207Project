using .Jisk.MonteCarlo

@testset "MC energy conservation" begin
    grid = build_grid(; N_r=32, N_phi=4, N_z=32,
                       r_min_AU=10.0, r_max_AU=100.0, z_max_AU=10.0,
                       T=Float32)

    ρ_host = Jisk.DiskProfile.build_density_array(grid)
    ρ_d = CuArray(ρ_host)

    κ_abs = 5.0f2
    κ_sca = 5.0f2
    N_p = Int64(200_000)

    res = run_mc!(grid, ρ_d; κ_abs=κ_abs, κ_sca=κ_sca, g=0.086, phase=:hg,
                  N_p=N_p, max_scatter=200, seed=UInt64(0xC0DE))

    n_abs  = Array(res.n_absorbed)[1]
    n_esc  = Array(res.n_escaped)[1]
    n_rr   = Array(res.n_rr_killed)[1]
    @test n_abs + n_esc + n_rr == N_p

    # Every packet terminates exactly once.
    # Deposited weights are nonnegative.
    E_abs_host = Array(res.E_abs)
    S_accum    = Array(res.S_accum)
    n_scat_h   = Array(res.n_scatters)[1]
    @test all(E_abs_host .>= 0)
    @test all(S_accum .>= 0)
    @test sum(E_abs_host) > 0
    @test n_scat_h > 0
    @test sum(S_accum) > 0
    # Scatter counter should match array-accumulated total (each scatter deposits w=1 unless RR).
    @test isapprox(sum(S_accum), Float32(n_scat_h); rtol=0.2)
end
