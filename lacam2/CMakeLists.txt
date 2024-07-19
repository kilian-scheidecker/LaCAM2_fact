cmake_minimum_required(VERSION 3.16)
project(lacam2)

# Gather source files
file(GLOB SRCS "./src/*.cpp")

# Find and set up OpenMP
find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
    set(OpenMP_LIB OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# Create the static library
add_library(${PROJECT_NAME} STATIC ${SRCS})

# Set the path to easy_profiler include directory
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/../third_party/easy_profiler/easy_profiler_core/include)

# Compile options
target_compile_options(${PROJECT_NAME} PUBLIC -O3 -Wall -g)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} INTERFACE ./include)

# Link EasyProfiler and OpenMP libraries
target_link_libraries(${PROJECT_NAME} PUBLIC easy_profiler ${OpenMP_LIB})
