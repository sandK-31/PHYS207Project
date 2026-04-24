@testset "Grid" begin
    grid = build_grid(; N_r=32, N_phi=16, N_z=32, T=Float64)

    @test length(grid.r_edges) == 33
    @test grid.r_edges[1] ≈ Constants.R_in_AU * Constants.AU
    @test grid.r_edges[end] ≈ Constants.R_out_AU * Constants.AU
    @test issorted(grid.r_edges)
    @test all(diff(grid.log_r_edges) .> 0)

    # Voxel volume closure: sum of all voxels = π (r_max^2 - r_min^2) · (z_max - z_min)
    V = voxel_volume_array(grid)
    V_total = sum(V)
    V_analytic = π * Float64(grid.r_max)^2 - π * Float64(grid.r_min)^2
    V_analytic *= Float64(grid.z_max - grid.z_min)
    @test isapprox(V_total, V_analytic; rtol=1e-10)

    # Index round-trip: place a point in a known voxel and check indices
    i_target, j_target, k_target = 7, 5, 20
    r_c   = 0.5 * (grid.r_edges[i_target] + grid.r_edges[i_target+1])
    phi_c = 0.5 * (grid.phi_edges[j_target] + grid.phi_edges[j_target+1])
    z_c   = 0.5 * (grid.z_edges[k_target] + grid.z_edges[k_target+1])
    x = r_c * cos(phi_c); y = r_c * sin(phi_c); z = z_c

    ir, iphi, iz = Grid.cyl_indices_from_cart(x, y, z,
                                              grid.log_r_edges, grid.N_r,
                                              grid.dphi, grid.N_phi,
                                              grid.z_min, grid.dz, grid.N_z)
    @test Int(ir)   == i_target
    @test Int(iphi) == j_target
    @test Int(iz)   == k_target

    # Outside-grid points return 0 in at least one index
    ir_out, _, _ = Grid.cyl_indices_from_cart(0.0, 0.0, 0.0,
                                              grid.log_r_edges, grid.N_r,
                                              grid.dphi, grid.N_phi,
                                              grid.z_min, grid.dz, grid.N_z)
    @test Int(ir_out) == 0

    _, _, iz_out = Grid.cyl_indices_from_cart(r_c, 0.0, grid.z_max * 2,
                                              grid.log_r_edges, grid.N_r,
                                              grid.dphi, grid.N_phi,
                                              grid.z_min, grid.dz, grid.N_z)
    @test Int(iz_out) == 0
end
