cmake_minimum_required(VERSION 3.16)
project(lacam2-project CXX)

# Add the lacam2 and argparse subdirectories
add_subdirectory(./lacam2)
add_subdirectory(./third_party/argparse)

# Set the path to easy_profiler
set(EASY_PROFILER_DIR ./third_party/easy_profiler)

# Add the easy_profiler include directory
include_directories(${EASY_PROFILER_DIR}/easy_profiler_core/include)

# Add the easy_profiler library
add_subdirectory(${EASY_PROFILER_DIR} ${CMAKE_BINARY_DIR}/easy_profiler_build)

# Add the main executable
add_executable(main main.cpp)

# Specify C++ standard
target_compile_features(main PUBLIC cxx_std_17)

# Link libraries to the main executable
target_link_libraries(main lacam2 argparse easy_profiler)

