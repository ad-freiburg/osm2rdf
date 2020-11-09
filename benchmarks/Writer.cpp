// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <benchmark/benchmark.h>

#include "osm2ttl/ttl/Format.h"
#include "osm2ttl/ttl/Writer.h"

static void Writer_NT_generateBlankNode(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateBlankNode();
  }
}
BENCHMARK(Writer_NT_generateBlankNode);

static void Writer_TTL_generateBlankNode(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateBlankNode();
  }
}
BENCHMARK(Writer_TTL_generateBlankNode);

static void Writer_QLEVER_generateBlankNode(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateBlankNode();
  }
}
BENCHMARK(Writer_QLEVER_generateBlankNode);

static void Writer_NT_generateLangTag(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateLangTag("de");
  }
}
BENCHMARK(Writer_NT_generateLangTag);

static void Writer_TTL_generateLangTag(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateLangTag("de");
  }
}
BENCHMARK(Writer_TTL_generateLangTag);

static void Writer_QLEVER_generateLangTag(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateLangTag("de");
  }
}
BENCHMARK(Writer_QLEVER_generateLangTag);

static void Writer_NT_generateIRI(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateIRI("wd", "Q42");
  }
}
BENCHMARK(Writer_NT_generateIRI);

static void Writer_TTL_generateIRI(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateIRI("wd", "Q42");
  }
}
BENCHMARK(Writer_TTL_generateIRI);

static void Writer_QLEVER_generateIRI(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateIRI("wd", "Q42");
  }
}
BENCHMARK(Writer_QLEVER_generateIRI);

static void Writer_NT_generateLiteral(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(Writer_NT_generateLiteral);

static void Writer_TTL_generateLiteral(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(Writer_TTL_generateLiteral);

static void Writer_QLEVER_generateLiteral(benchmark::State& state) {
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(Writer_QLEVER_generateLiteral);

static void Writer_QLEVER_generateLiteral_a(benchmark::State& state) {
  std::string s(state.range(0), 'a');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::generateLiteral(s, "");
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_generateLiteral_a)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();


static void Writer_QLEVER_utf8Codepoint_a(benchmark::State& state) {
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_a)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();


static void Writer_QLEVER_utf8Codepoint_A(benchmark::State& state) {
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_A)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();


static void Writer_QLEVER_utf8Codepoint_AT(benchmark::State& state) {
  std::string s(state.range(0), '@');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_AT)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void Writer_QLEVER_utf8Codepoint_0xFAFA(benchmark::State& state) {
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
BENCHMARK(Writer_QLEVER_utf8Codepoint_0xFAFA)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_a(benchmark::State& state) {
  std::string s(state.range(0), 'a');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_a)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_A(benchmark::State& state) {
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_A)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_AT(benchmark::State& state) {
  std::string s(state.range(0), '@');
  for (auto _ : state) {
    osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_AT)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_0xFAFA(benchmark::State& state) {
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
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_0xFAFA)
->RangeMultiplier(2)->Range(1<<4, 1<<11)->Complexity();