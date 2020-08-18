set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/google/benchmark" "vendor/google/benchmark")

set_target_properties(benchmark PROPERTIES FOLDER vendor)
set_target_properties(benchmark_main PROPERTIES FOLDER vendor)

macro(package_add_benchmark BENCHMARKNAME)
    # create an executable in which the benchmarks will be stored
    add_executable(${BENCHMARKNAME} ${ARGN})
    target_link_libraries(${BENCHMARKNAME} benchmark::benchmark)
    set_target_properties(${BENCHMARKNAME} PROPERTIES FOLDER benchmarks)
endmacro()

package_add_benchmark(benchmarks benchmarks.cpp)
