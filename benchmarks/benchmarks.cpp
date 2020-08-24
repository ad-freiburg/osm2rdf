// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <benchmark/benchmark.h>
#include <osm2ttl/config/Config.h>

#include "osm2ttl/ttl/Format.h"
#include "osm2ttl/ttl/Writer.h"

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

static void OSM2TT_TTL_OUTPUTFORMAT_BLANKNODE_NT(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateBlankNode();
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_BLANKNODE_NT);

static void OSM2TT_TTL_OUTPUTFORMAT_BLANKNODE_TTL(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateBlankNode();
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_BLANKNODE_TTL);

static void OSM2TT_TTL_OUTPUTFORMAT_BLANKNODE_QLEVER(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateBlankNode();
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_BLANKNODE_QLEVER);

static void OSM2TT_TTL_OUTPUTFORMAT_LANGTAG_NT(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateLangTag("de");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LANGTAG_NT);

static void OSM2TT_TTL_OUTPUTFORMAT_LANGTAG_TTL(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateLangTag("de");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LANGTAG_TTL);

static void OSM2TT_TTL_OUTPUTFORMAT_LANGTAG_QLEVER(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateLangTag("de");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LANGTAG_QLEVER);

static void OSM2TT_TTL_OUTPUTFORMAT_IRI_NT(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateIRI("wd", "Q42");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_IRI_NT);

static void OSM2TT_TTL_OUTPUTFORMAT_IRI_TTL(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateIRI("wd", "Q42");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_IRI_TTL);

static void OSM2TT_TTL_OUTPUTFORMAT_IRI_QLEVER(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateIRI("wd", "Q42");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_IRI_QLEVER);

static void OSM2TT_TTL_OUTPUTFORMAT_LITERAL_NT(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_NT);

static void OSM2TT_TTL_OUTPUTFORMAT_LITERAL_TTL(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_TTL);

static void OSM2TT_TTL_OUTPUTFORMAT_LITERAL_QLEVER(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_QLEVER);