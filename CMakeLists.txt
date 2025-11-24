cmake_minimum_required(VERSION 3.25)
project(luceo LANGUAGES CXX)

# Use C++20 as language standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Glob all the source files
file(GLOB_RECURSE PROJECT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

# Add executable
add_executable(luceo ${PROJECT_SOURCES})

# Add subdirectories
add_subdirectory("extern")