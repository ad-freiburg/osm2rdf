// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "benchmark/benchmark.h"

#include <numeric>

#include "omp.h"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/GeometryHandler.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/DirectedGraph.h"

static void GeometryHandler_reduceDAG_Line_MinimalConnections(benchmark::State& state) {
  osm2ttl::util::DirectedGraph dg{};
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    dg.addEdge(vertices[i], vertices[i + 1]);
  }
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};
  for (auto _ : state) {
    geometryHandler.reduceDAG(dg, false);
  }
}
BENCHMARK(GeometryHandler_reduceDAG_Line_MinimalConnections)
->RangeMultiplier(2)->Range(1<<1, 1<<8)->Complexity();

static void GeometryHandler_reduceDAG_Line_MaximalConnections(benchmark::State& state) {
  osm2ttl::util::DirectedGraph dg{};
  std::vector<uint64_t> vertices(state.range(0));
  std::iota(std::begin(vertices), std::end(vertices), 0);
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    for (size_t j = i; j < vertices.size() - 1; ++j) {
      dg.addEdge(vertices[i], vertices[j + 1]);
    }
  }
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};
  for (auto _ : state) {
    geometryHandler.reduceDAG(dg, false);
  }
}
BENCHMARK(GeometryHandler_reduceDAG_Line_MaximalConnections)
->RangeMultiplier(2)->Range(1<<1, 1<<8)->Complexity();