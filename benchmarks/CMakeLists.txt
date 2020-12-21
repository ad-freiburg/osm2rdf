set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/google/benchmark" "vendor/google/benchmark")

set_target_properties(benchmark PROPERTIES FOLDER vendor)
set_target_properties(benchmark_main PROPERTIES FOLDER vendor)

macro(package_add_benchmark BENCHMARKNAME)
    # create an executable in which the benchmarks will be stored
    add_executable(${BENCHMARKNAME} ${ARGN})
    target_link_libraries(${BENCHMARKNAME} PRIVATE benchmark::benchmark_main)
    target_link_libraries(${BENCHMARKNAME} PRIVATE osm2ttl_library)
    set_target_properties(${BENCHMARKNAME} PROPERTIES FOLDER benchmarks)
    # register benchmark for global build target
    add_dependencies(build_benchmarks ${BENCHMARKNAME})
    add_dependencies(run_benchmarks run_${BENCHMARKNAME})
    add_custom_target(run_${BENCHMARKNAME} COMMAND ${BENCHMARKNAME} DEPENDS ${BENCHMARKNAME} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/benchmarks)
endmacro()

add_custom_target(build_benchmarks)
add_custom_target(run_benchmarks)
package_add_benchmark(BaselinesBenchmark Baselines.cpp)
package_add_benchmark(DirectedGraphBenchmark DirectedGraph.cpp)
package_add_benchmark(DirectedAcyclicGraphBenchmark DirectedAcyclicGraph.cpp)
package_add_benchmark(OpenMPBenchmark OpenMP.cpp)
package_add_benchmark(WriterBenchmark Writer.cpp)
