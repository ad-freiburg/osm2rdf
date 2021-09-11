// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2rdf/util/DirectedAcyclicGraph.h"

#include <iostream>
#include <numeric>

#include "benchmark/benchmark.h"
#include "osm2rdf/util/DirectedGraph.h"

// ____________________________________________________________________________
static void DirectedAcyclicGraph_reduceDAG_Line_MinimalConnections(
    benchmark::State& state) {
  osm2rdf::util::DirectedGraph<u_int16_t> dg{};
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  dg.prepareFindSuccessorsFast();

  for (auto _ : state) {
    osm2rdf::util::reduceDAG(dg, false);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedAcyclicGraph_reduceDAG_Line_MinimalConnections)
    ->RangeMultiplier(2)
    ->Range(1U << 1U, 1U << 8U)
    ->Complexity();

// ____________________________________________________________________________
static void DirectedAcyclicGraph_reduceDAG_Line_MaximalConnections(
    benchmark::State& state) {
  osm2rdf::util::DirectedGraph<u_int16_t> dg{};
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    for (size_t j = i; j < vertices.size() - 1; ++j) {
      dg.addEdge(vertices[i], vertices[j + 1]);
    }
  }
  dg.prepareFindSuccessorsFast();

  for (auto _ : state) {
    osm2rdf::util::reduceDAG(dg, false);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedAcyclicGraph_reduceDAG_Line_MaximalConnections)
    ->RangeMultiplier(2)
    ->Range(1U << 1U, 1U << 4U)
    ->Complexity();
