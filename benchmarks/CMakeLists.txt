# Copyright 2020, University of Freiburg
# Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

# This file is part of osm2rdf.
#
# osm2rdf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# osm2rdf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/google/benchmark" "vendor/google/benchmark")

set_target_properties(benchmark PROPERTIES FOLDER vendor)
set_target_properties(benchmark_main PROPERTIES FOLDER vendor)

macro(package_add_benchmark BENCHMARKNAME)
    # create an executable in which the benchmarks will be stored
    add_executable(${BENCHMARKNAME} ${ARGN})
    target_link_libraries(${BENCHMARKNAME} PRIVATE benchmark::benchmark_main)
    target_link_libraries(${BENCHMARKNAME} PRIVATE osm2rdf_library)
    set_target_properties(${BENCHMARKNAME} PROPERTIES FOLDER benchmarks)
    # register benchmark for global build target
    add_dependencies(build_benchmarks ${BENCHMARKNAME})
    add_dependencies(run_benchmarks run_${BENCHMARKNAME})
    add_custom_target(run_${BENCHMARKNAME} COMMAND ${BENCHMARKNAME} DEPENDS ${BENCHMARKNAME} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/benchmarks)
endmacro()

add_custom_target(build_benchmarks)
add_custom_target(run_benchmarks)
package_add_benchmark(BaselinesBenchmark Baselines.cpp)
package_add_benchmark(DirectedGraphBenchmark util/DirectedGraph.cpp)
package_add_benchmark(DirectedAcyclicGraphBenchmark util/DirectedAcyclicGraph.cpp)
package_add_benchmark(OpenMPBenchmark OpenMP.cpp)
package_add_benchmark(WriterBenchmark ttl/Writer.cpp)
