cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
# set(CMAKE_CXX_COMPILER
# "/home/sanderk/.conda/pkgs/gxx-13.3.0-h9576a4e_2/bin/g++")
project(WLS_ML_cube)
set(CMAKE_CXX_STANDARD 17)
include(FindROOT.cmake)
include(FindGeant4.cmake)

find_package(Geant4 REQUIRED)
find_package(ROOT REQUIRED)

# ----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
find_package(Geant4 REQUIRED)
# ----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions Setup include
# directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)
# target_link_libraries(main ${SimEvent_LIBRARIES})
# ----------------------------------------------------------------------------
# Locate sources and headers for this project NB: headers are included so they
# will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# ----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
if(ROOT_FOUND)
  add_definitions(-DG4ANALYSIS_USE_ROOT)
  include(${Geant4_USE_FILE})
  include_directories(${ROOT_INCLUDE_DIR} ${Geant4_INCLUDE_DIR}
                      ${PROJECT_SOURCE_DIR}/include)
else()
  include(${Geant4_USE_FILE})
  include_directories(${PROJECT_SOURCE_DIR}/include)
endif()

# ----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(laser laser.cc ${sources} ${headers})
target_link_libraries(laser ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

# ----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we build
# fission. This is so that we can run the executable directly because it relies
# on these scripts being in the current working directory.
#
set(my_SCRIPTS vis.mac init_vis.mac run1.mac)

file(GLOB_RECURSE MACRO_FILES "*.mac" "*.txt")

file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

foreach(_script ${my_SCRIPTS})
  configure_file(${PROJECT_SOURCE_DIR}/${_script}
                 ${PROJECT_BINARY_DIR}/${_script} COPYONLY)
endforeach()

install(TARGETS laser DESTINATION .)
