// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <benchmark/benchmark.h>

#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/OutputFormat.h"

static void BM_StringCreation(benchmark::State& state) {
  for (auto _ : state) {
    std::string empty_string;
  }
}
BENCHMARK(BM_StringCreation);

static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  for (auto _ : state) {
    std::string copy(x);
  }
}
BENCHMARK(BM_StringCopy);

static void OSM2TT_TTL_OUTPUTFORMAT_IRI_NT(benchmark::State& state) {
  osm2ttl::ttl::IRI iri("wd", "Q42");
  osm2ttl::ttl::OutputFormat of = osm2ttl::ttl::OutputFormat::NT;
  for (auto _ : state) {
    of.format(iri);
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_IRI_NT);

static void OSM2TT_TTL_OUTPUTFORMAT_IRI_TTL(benchmark::State& state) {
  osm2ttl::ttl::OutputFormat of = osm2ttl::ttl::OutputFormat::TTL;
  osm2ttl::ttl::IRI iri("wd", "Q42");
  for (auto _ : state) {
    of.format(iri);
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_IRI_TTL);

static void OSM2TT_TTL_OUTPUTFORMAT_IRI_QLEVER(benchmark::State& state) {
  osm2ttl::ttl::OutputFormat of = osm2ttl::ttl::OutputFormat::QLEVER;
  osm2ttl::ttl::IRI iri("wd", "Q42");
  for (auto _ : state) {
    of.format(iri);
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_IRI_QLEVER);

static void OSM2TT_TTL_OUTPUTFORMAT_LITERAL_NT(benchmark::State& state) {
  osm2ttl::ttl::OutputFormat of = osm2ttl::ttl::OutputFormat::NT;
  osm2ttl::ttl::Literal literal("Lorem ipsum dolor sit amet");
  for (auto _ : state) {
    of.format(literal);
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_NT);

static void OSM2TT_TTL_OUTPUTFORMAT_LITERAL_TTL(benchmark::State& state) {
  osm2ttl::ttl::OutputFormat of = osm2ttl::ttl::OutputFormat::TTL;
  osm2ttl::ttl::Literal literal("Lorem ipsum dolor sit amet");
  for (auto _ : state) {
    of.format(literal);
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_TTL);

static void OSM2TT_TTL_OUTPUTFORMAT_LITERAL_QLEVER(benchmark::State& state) {
  osm2ttl::ttl::OutputFormat of = osm2ttl::ttl::OutputFormat::QLEVER;
  osm2ttl::ttl::Literal literal("Lorem ipsum dolor sit amet");
  for (auto _ : state) {
    of.format(literal);
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_QLEVER);