using .Jisk.RayTrace

@testset "Ray-tracer sanity" begin
    grid = build_grid(; N_r=32, N_phi=4, N_z=32, T=Float32)
    ρ_d = CuArray(zeros(Float32, grid.N_r, grid.N_phi, grid.N_z))
    S_d = CuArray(zeros(Float32, grid.N_r, grid.N_phi, grid.N_z))

    obs = ObserverParams(; inclination=Float32(π/3), azimuth=0f0,
                          N_px=Int32(32), N_py=Int32(32),
                          fov_cm=Float32(250 * Constants.AU),
                          n_steps=Int32(128))

    # Empty disk → zero image
    img = render_image(grid, ρ_d, S_d, obs;
                       κ_abs=1.0, κ_sca=0.0, λ_um=1.0,
                       T_mid_r=nothing,
                       include_thermal=false, include_scatter=true)
    @test all(Array(img) .== 0f0)

    # Uniform density + uniform S with pure scattering: thermal off.
    # Output intensity should be nonzero and bounded.
    fill!(ρ_d, 1.0f-18)
    fill!(S_d, 1.0f0)
    img2 = render_image(grid, ρ_d, S_d, obs;
                        κ_abs=0.0, κ_sca=1.0f3, λ_um=1.0,
                        T_mid_r=nothing,
                        include_thermal=false, include_scatter=true)
    @test all(Array(img2) .>= 0f0)
    @test maximum(Array(img2)) > 0f0

    # Thermal-only on a uniform-ρ cube: should also be nonzero and positive
    T_mid_r = fill(100.0f0, Int(grid.N_r))
    img3 = render_image(grid, ρ_d, S_d, obs;
                        κ_abs=1.0f3, κ_sca=0.0, λ_um=10.0,
                        T_mid_r=T_mid_r,
                        include_thermal=true, include_scatter=false)
    @test all(Array(img3) .>= 0f0)
    @test maximum(Array(img3)) > 0f0
end
