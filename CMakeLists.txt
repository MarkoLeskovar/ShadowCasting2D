cmake_minimum_required(VERSION 3.7.0)

# Set a projec name and programming language
project(ShadowCasting2D CXX)

# Set the C++ standard you need
set(CMAKE_CXX_STANDARD 17)

# Include extra CMake utilities that have no place in the
# main CMakeLists. This imports the following functions:
# - link
# - build_time_copy_target
# - install_library
# - install_executable
include("${CMAKE_CURRENT_SOURCE_DIR}/scripts/utilities.cmake")

# Ask CMake to generate a file that stores
# the source files' relations
# (helps your language server with highlighting).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json")

# This will add the install path to the rpath
# of the installed binaries to make dynamic linking work.
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}:$ORIGIN")

# This will force all classes to be exported when compiling
# libraries (makes all of your classes/functions/variables
# accessible outside the library automatically).
# Review this if C++20 modules ever get supported in CMake.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Compiler flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic") # reject compiler-specific code
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # Turn on all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") # Force position independent code
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror") # Treat all warnings as errors (turn this off if thirdparty depencies start throwing errors)


# Windows needs libraries to be in the same directory
# as the binaries they're linked to (or in the system binary directory),
# so we'll copy all built binaries into the same directory later.
# For now, create that directory (at build time).
set(build_bin_directory "${CMAKE_BINARY_DIR}/bin")
add_custom_target(build-time-make-directory ALL
                  COMMAND ${CMAKE_COMMAND} -E
                  make_directory "${build_bin_directory}")

# Same goes for the install step
set(install_bin_directory "${CMAKE_INSTALL_PREFIX}/bin")
install(DIRECTORY DESTINATION "${install_bin_directory}")

# Make the 'inc' directory visible to all sources
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

# Collect sources
file(GLOB_RECURSE headers "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp") # library headers
file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp") # library sources
file(GLOB_RECURSE driver_sources "${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.cpp") # drivers

# Build a shared library fron sources in 'inc' and 'src'.
# Note: Visual Studio will only group files that you pass
# to add_library here, so you might want to pass the headers
# in there too, even though they aren't necessary. It's a
# workaround so read up on what the optimal solution is if
# you don't like it.
add_library(${PROJECT_NAME} SHARED ${sources})
build_time_copy_target(${PROJECT_NAME} "${build_bin_directory}") # Copy library to the common bin directory
install_library(${PROJECT_NAME} "${install_bin_directory}") # 'Copy' library to the common install bin directory


# Build an executable for each source in 'drivers'
foreach(driver ${driver_sources})
    # Define the executable's name as the stub of the source
    get_filename_component(driver_name ${driver} NAME_WE)

    # Build the driver and link it to the library.
    add_executable(${driver_name} ${driver})
    link(${driver_name} ${PROJECT_NAME})
    build_time_copy_target(${driver_name} "${build_bin_directory}") # Copy executable to the common bin directory
    install_library(${driver_name} "${install_bin_directory}") # 'Copy' library to the common install bin directory
endforeach()
