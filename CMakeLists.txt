cmake_minimum_required(VERSION 3.16)
project(lacam2_project CXX)

# Add the easy_profiler subdirectory first
add_subdirectory(third_party/easy_profiler)
add_subdirectory(third_party/json)

# Add the lacam2 and argparse subdirectories
add_subdirectory(lacam2)
add_subdirectory(third_party/argparse)


# Find and set up OpenMP
find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
  set(OpenMP_LIB OpenMP::OpenMP_CXX)
else()
  message(FATAL_ERROR "OpenMP not found")
endif()

# Add the main executable
add_executable(main main.cpp)

# Specify C++ standard
target_compile_features(main PUBLIC cxx_std_17)

# Link libraries to the main executable
target_link_libraries(main lacam2 argparse easy_profiler json ${OpenMP_LIB})
