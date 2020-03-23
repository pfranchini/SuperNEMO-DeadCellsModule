# - Basic CMake setup
# Check version meets our requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(DeadCellsModule)

find_package(Falaise REQUIRED)

# Build a dynamic library from our sources
add_library(DeadCellsModule SHARED DeadCellsModule.h DeadCellsModule.cpp lib/FLSimulateCommandLine.h lib/FLSimulateErrors.h lib/FLSimulateArgs.h lib/FLSimulateArgs.cc)

# Build an executable from our sources
#add_executable(DeadCellsModule DeadCellsModule.cpp DeadCellsModule.h  FLSimulateArgs.h  FLSimulateCommandLine.h  FLSimulateErrors.h FLSimulateArgs.cc)

# Link it to the FalaiseModule library
# This ensures the correct compiler flags, include paths
# and linker flags are applied to our dynamic library.
# - Explicit link to ROOT Physics library which is not linked
#   by flreconstruct
target_link_libraries(DeadCellsModule
  PUBLIC
    Falaise::FalaiseModule
    Bayeux::Bayeux_mctools_geant4		
    ${ROOT_Physics_LIBRARY}
    )

# Configure example pipeline script for use from the build dir
configure_file("DeadCellsModule.conf.in" "DeadCellsModule.conf" @ONLY)

file(COPY official-2.0.0_only_calibration.conf DESTINATION ${PROJECT_BINARY_DIR})
file(COPY official-2.0.0_from_CAT.conf DESTINATION ${PROJECT_BINARY_DIR})

# Add a basic test: simulated a brio, reconstruct up to calibration and runs the module on it
enable_testing()

# - Simulate
add_test(NAME testDeadCellsModule_simulate
  COMMAND Falaise::flsimulate -o test-sim.brio
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

# - Reconstruct up to calibration
add_test(NAME testDeadCellsModule_reconstruct
  COMMAND Falaise::flreconstruct -i test-sim.brio -p official-2.0.0_only_calibration.conf -o test-cal.brio
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

# - Run Module
add_test(NAME testDeadCellsModule_module
  COMMAND Falaise::flreconstruct -i test-cal.brio -p DeadCellsModule.conf
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

