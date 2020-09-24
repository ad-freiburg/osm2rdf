// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <benchmark/benchmark.h>

#include "osm2ttl/ttl/Format.h"
#include "osm2ttl/ttl/Writer.h"

static void UINT64T_TO_STRING(benchmark::State& state) {
  uint64_t x = state.range(0);
  for (auto _ : state) {
    std::string dummy = std::to_string(x);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(UINT64T_TO_STRING)
->RangeMultiplier(2)->Range(1<<0, 1<<30)->Complexity();

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
  std::string s(state.range(0), 'a');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateLiteral(s, "");
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_OUTPUTFORMAT_LITERAL_QLEVER)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();


static void OSM2TT_TTL_WRITER_ENCODE_UTFCODEPOINT_A(benchmark::State& state) {
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_WRITER_ENCODE_UTFCODEPOINT_A)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();


static void OSM2TT_TTL_WRITER_ENCODE_UTFCODEPOINT_AT(benchmark::State& state) {
  std::string s(state.range(0), '@');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_WRITER_ENCODE_UTFCODEPOINT_AT)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void OSM2TT_TTL_WRITER_ENCODE_UTFCODEPOINT_UTF8(benchmark::State& state) {
  std::string s{ u8"\ufafa"};
  s.reserve(s.length() * state.range(0));
  for (auto i = 1; i < state.range(0); ++i) {
    s += std::string{ u8"\ufafa"};
  }
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_WRITER_ENCODE_UTFCODEPOINT_UTF8)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void OSM2TT_TTL_WRITER_ENCODE_PN_LOCAL_A(benchmark::State& state) {
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_WRITER_ENCODE_PN_LOCAL_A)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void OSM2TT_TTL_WRITER_ENCODE_PN_LOCAL_AT(benchmark::State& state) {
  std::string s(state.range(0), '@');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_WRITER_ENCODE_PN_LOCAL_AT)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void OSM2TT_TTL_WRITER_ENCODE_PN_LOCAL_UTF8(benchmark::State& state) {
  std::string s{ u8"\ufafa"};
  s.reserve(s.length() * state.range(0));
  for (auto i = 1; i < state.range(0); ++i) {
    s += std::string{ u8"\ufafa"};
  }
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(OSM2TT_TTL_WRITER_ENCODE_PN_LOCAL_UTF8)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void DUMMY(benchmark::State& state) {
  for (auto _ : state) {
    size_t s;
    for (size_t i = 0; i < state.range(0); ++i) {
      for (size_t j = 0; j < state.range(0); ++j) {
        s += j;
      }
    }
    benchmark::DoNotOptimize(s--);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DUMMY)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();