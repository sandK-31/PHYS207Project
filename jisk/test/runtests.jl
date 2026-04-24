using Test

push!(LOAD_PATH, joinpath(@__DIR__, "..", "src"))
include(joinpath(@__DIR__, "..", "src", "Jisk.jl"))
using .Jisk
using .Jisk.Constants
using .Jisk.Grid
using .Jisk.DiskProfile
using .Jisk.OpacityTable
using .Jisk.Phase

using CUDA
const HAS_GPU = try CUDA.functional() catch _; false end

@testset "Jisk" begin
    include("test_grid.jl")
    include("test_phase.jl")
    include("test_opacity.jl")
    include("test_disk.jl")
    if HAS_GPU
        include("test_mc_homogeneous.jl")
        include("test_raytrace.jl")
        include("test_energy.jl")
    else
        @info "CUDA not functional — GPU tests skipped."
    end
end
