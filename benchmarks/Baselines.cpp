// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "benchmark/benchmark.h"

static void UINT64T_TO_STRING(benchmark::State& state) {
  uint64_t x = state.range(0);
  for (auto _ : state) {
    std::string dummy = std::to_string(x);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(UINT64T_TO_STRING)
->RangeMultiplier(2)->Range(1<<0, 1<<30)->Complexity();

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