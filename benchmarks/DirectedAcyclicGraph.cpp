// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <iostream>
#include <numeric>

#include "benchmark/benchmark.h"
#include "osm2ttl/util/DirectedAcyclicGraph.h"
#include "osm2ttl/util/DirectedGraph.h"

static void DirectedAcyclicGraph_reduceDAG_Line_MinimalConnections(
    benchmark::State& state) {
  osm2ttl::util::DirectedGraph<u_int16_t> dg{};
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  dg.prepareFindSuccessorsFast();

  for (auto _ : state) {
    osm2ttl::util::reduceDAG(dg, false);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedAcyclicGraph_reduceDAG_Line_MinimalConnections)
->RangeMultiplier(2)->Range(1U << 1U, 1U << 8U)->Complexity();

static void DirectedAcyclicGraph_reduceDAG_Line_MaximalConnections(
    benchmark::State& state) {
  osm2ttl::util::DirectedGraph<u_int16_t> dg{};
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    for (size_t j = i; j < vertices.size() - 1; ++j) {
      dg.addEdge(vertices[i], vertices[j + 1]);
    }
  }
  dg.prepareFindSuccessorsFast();

  for (auto _ : state) {
    osm2ttl::util::reduceDAG(dg, false);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DirectedAcyclicGraph_reduceDAG_Line_MaximalConnections)
->RangeMultiplier(2)->Range(1U << 1U, 1U << 4U)->Complexity();