// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "benchmark/benchmark.h"

#include "osm2ttl/ttl/Format.h"
#include "osm2ttl/ttl/Writer.h"

static void Writer_NT_generateBlankNode(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  for (auto _ : state) {
    w.generateBlankNode();
  }
}
BENCHMARK(Writer_NT_generateBlankNode);

static void Writer_TTL_generateBlankNode(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config};
  for (auto _ : state) {
    w.generateBlankNode();
  }
}
BENCHMARK(Writer_TTL_generateBlankNode);

static void Writer_QLEVER_generateBlankNode(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  for (auto _ : state) {
    w.generateBlankNode();
  }
}
BENCHMARK(Writer_QLEVER_generateBlankNode);

static void Writer_NT_generateLangTag(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  for (auto _ : state) {
    w.generateLangTag("de");
  }
}
BENCHMARK(Writer_NT_generateLangTag);

static void Writer_TTL_generateLangTag(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config};
  for (auto _ : state) {
    w.generateLangTag("de");
  }
}
BENCHMARK(Writer_TTL_generateLangTag);

static void Writer_QLEVER_generateLangTag(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  for (auto _ : state) {
    w.generateLangTag("de");
  }
}
BENCHMARK(Writer_QLEVER_generateLangTag);

static void Writer_NT_generateIRI(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  for (auto _ : state) {
    w.generateIRI("wd", "Q42");
  }
}
BENCHMARK(Writer_NT_generateIRI);

static void Writer_TTL_generateIRI(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config};
  for (auto _ : state) {
    w.generateIRI("wd", "Q42");
  }
}
BENCHMARK(Writer_TTL_generateIRI);

static void Writer_QLEVER_generateIRI(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  for (auto _ : state) {
    w.generateIRI("wd", "Q42");
  }
}
BENCHMARK(Writer_QLEVER_generateIRI);

static void Writer_NT_generateLiteral(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  for (auto _ : state) {
    w.generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(Writer_NT_generateLiteral);

static void Writer_TTL_generateLiteral(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config};
  for (auto _ : state) {
    w.generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(Writer_TTL_generateLiteral);

static void Writer_QLEVER_generateLiteral(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  for (auto _ : state) {
    w.generateLiteral("Lorem ipsum dolor sit amet", "");
  }
}
BENCHMARK(Writer_QLEVER_generateLiteral);

static void Writer_QLEVER_generateLiteral_a(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), 'a');
  for (auto _ : state) {
    w.generateLiteral(s, "");
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_generateLiteral_a)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();


static void Writer_QLEVER_utf8Codepoint_a(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    w.utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_a)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();


static void Writer_QLEVER_utf8Codepoint_A(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    w.utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_A)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();


static void Writer_QLEVER_utf8Codepoint_AT(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), '@');
  for (auto _ : state) {
    w.utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_AT)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();

static void Writer_QLEVER_utf8Codepoint_0xFAFA(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s{ u8"\ufafa"};
  s.reserve(s.length() * state.range(0));
  for (auto i = 1; i < state.range(0); ++i) {
    s += std::string{ u8"\ufafa"};
  }
  for (auto _ : state) {
    w.utf8Codepoint(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_utf8Codepoint_0xFAFA)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_a(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), 'a');
  for (auto _ : state) {
    w.encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_a)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_A(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), 'A');
  for (auto _ : state) {
    w.encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_A)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_AT(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s(state.range(0), '@');
  for (auto _ : state) {
    w.encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_AT)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();

static void Writer_QLEVER_encodePN_LOCAL_0xFAFA(benchmark::State& state) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config};
  std::string s{ u8"\ufafa"};
  s.reserve(s.length() * state.range(0));
  for (auto i = 1; i < state.range(0); ++i) {
    s += std::string{ u8"\ufafa"};
  }
  for (auto _ : state) {
    w.encodePN_LOCAL(s);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(Writer_QLEVER_encodePN_LOCAL_0xFAFA)
->RangeMultiplier(2)->Range(1U<<4U, 1U<<11U)->Complexity();