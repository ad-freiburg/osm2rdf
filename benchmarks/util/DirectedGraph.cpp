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

#include "osm2rdf/util/DirectedGraph.h"

#include <numeric>

#include "benchmark/benchmark.h"

// ____________________________________________________________________________
static void DirectedGraph_findSuccessors_First(benchmark::State& state) {
  osm2rdf::util::DirectedGraph<uint16_t> dg;
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  std::vector<uint16_t> x;
  for (auto _ : state) {
    x = dg.findSuccessors(0);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedGraph_findSuccessors_First)
    ->RangeMultiplier(2)
    ->Range(1U << 1U, 1U << 10U)
    ->Complexity();

// ____________________________________________________________________________
static void DirectedGraph_findSuccessors_Last(benchmark::State& state) {
  osm2rdf::util::DirectedGraph<uint16_t> dg;
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  std::vector<uint16_t> x;
  for (auto _ : state) {
    x = dg.findSuccessors(vertices.size()-1);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedGraph_findSuccessors_Last)
    ->RangeMultiplier(2)
    ->Range(1U << 1U, 1U << 10U)
    ->Complexity();

// ____________________________________________________________________________
static void DirectedGraph_findSuccessorsFast_First(benchmark::State& state) {
  osm2rdf::util::DirectedGraph<uint16_t> dg;
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  dg.prepareFindSuccessorsFast();
  std::vector<uint16_t> x;
  for (auto _ : state) {
    x = dg.findSuccessorsFast(0);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedGraph_findSuccessorsFast_First)
    ->RangeMultiplier(2)
    ->Range(1U << 1U, 1U << 10U)
    ->Complexity();

static void DirectedGraph_findSuccessorsFast_Last(benchmark::State& state) {
  osm2rdf::util::DirectedGraph<uint16_t> dg;
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  dg.prepareFindSuccessorsFast();
  std::vector<uint16_t> x;
  for (auto _ : state) {
    x = dg.findSuccessorsFast(vertices.size()-1);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedGraph_findSuccessorsFast_Last)
    ->RangeMultiplier(2)
    ->Range(1U << 1U, 1U << 10U)
    ->Complexity();
