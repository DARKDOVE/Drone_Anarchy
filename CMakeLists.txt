# Set CMake minimum version
cmake_minimum_required (VERSION 3.10.2)
 
# Set project name
project (DroneAnarchy)
 
# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)
 
# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)
 
# Define target name
set (TARGET_NAME DroneAnarchy)
 
# Define source files
define_source_files (GLOB_CPP_PATTERNS Source/*.cpp GLOB_H_PATTERNS Source/*.h)

define_resource_dirs(
    GLOB_PATTERNS
    "${CMAKE_SOURCE_DIR}/bin/CoreData"
    "${CMAKE_SOURCE_DIR}/bin/GameData"
    "${CMAKE_SOURCE_DIR}/bin/GameLogic"
)

# Setup target with resource copying
setup_main_executable ()