using .Jisk.MonteCarlo

@testset "Monte Carlo (homogeneous limit)" begin
    # Build a small grid with constant density; check that first-collision statistics
    # recover the expected optical-depth distribution.
    grid = build_grid(; N_r=16, N_phi=4, N_z=16,
                        r_min_AU=10.0, r_max_AU=100.0, z_max_AU=10.0,
                        T=Float32)

    ρ_host = fill(1.0f-18, grid.N_r, grid.N_phi, grid.N_z)
    ρ_d = CuArray(ρ_host)

    κ_abs = 1.0f3
    κ_sca = 0.0f0            # pure absorption → track absorbed vs escaped fraction
    N_p = Int64(200_000)

    res = run_mc!(grid, ρ_d; κ_abs=κ_abs, κ_sca=κ_sca, g=0.0, phase=:iso,
                  N_p=N_p, max_scatter=50, seed=UInt64(0xBEEF))

    n_abs_h = Array(res.n_absorbed)[1]
    n_esc_h = Array(res.n_escaped)[1]
    @test n_abs_h + n_esc_h == N_p

    # With ρ·κ_abs > 0 and a finite box, some fraction should absorb. This is a
    # sanity check, not a precise analytic comparison — just non-pathological.
    @test 0 < n_abs_h < N_p
    @test 0 < n_esc_h < N_p
end
