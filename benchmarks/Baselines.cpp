// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "benchmark/benchmark.h"

static void UINT32T_TO_STRING_std_to_string(benchmark::State& state) {
  uint32_t x = state.range(0);
  for (auto _ : state) {
    std::string dummy = std::to_string(x);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(UINT32T_TO_STRING_std_to_string)
    ->RangeMultiplier(2)
    ->Range(1U << 0U, 1U << 30U)
    ->Complexity();

static void UINT64T_TO_STRING_std_to_string(benchmark::State& state) {
  uint64_t x = state.range(0);
  for (auto _ : state) {
    std::string dummy = std::to_string(x);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(UINT64T_TO_STRING_std_to_string)
    ->RangeMultiplier(2)
    ->Range(1U << 0U, 1U << 30U)
    ->Complexity();

static void DUMMY(benchmark::State& state) {
  for (auto _ : state) {
    size_t s = 0;
    for (int64_t i = 0; i < state.range(0); ++i) {
      for (int64_t j = 0; j < state.range(0); ++j) {
        s += j;
      }
    }
    benchmark::DoNotOptimize(s--);
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(DUMMY)->RangeMultiplier(2)->Range(1U << 4U, 1U << 11U)->Complexity();