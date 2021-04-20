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

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

typedef std::chrono::duration<double, std::milli> timingInfo;

// ____________________________________________________________________________
void run(size_t n, omp_sched_t st, int ss) {
  omp_set_schedule(st, ss);
  std::vector<int> iterations(omp_get_max_threads(), 0);
  std::vector<timingInfo> timing(omp_get_max_threads());
  std::vector<std::vector<int>> result(omp_get_max_threads());
  std::vector<int> input(n);
  std::iota(std::begin(input), std::end(input), 0);
  std::reverse(input.begin(), input.end());
  auto start = std::chrono::steady_clock::now();

#pragma omp parallel shared(iterations, input, result, timing) default(none)
  {
    auto innerStart = std::chrono::steady_clock::now();
#pragma omp for schedule(runtime)
    for (size_t i = 0; i < input.size(); i++) {
      iterations[omp_get_thread_num()]++;
      result[omp_get_thread_num()].push_back(input[i]);
      // Higher values sleep longer -> simulate different execution times...
      std::this_thread::sleep_for(std::chrono::nanoseconds(input[i] * 1000));
      // ... with some additional disturbance
      if (input[i] % 100 == 0) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10'000'000));
      }
    }
    auto innerEnd = std::chrono::steady_clock::now();
    timing[omp_get_thread_num()] =
        std::chrono::duration_cast<timingInfo>(innerEnd - innerStart);
  }
  auto end = std::chrono::steady_clock::now();
  timingInfo dur = std::chrono::duration_cast<timingInfo>(end - start);
  std::ostringstream stringStream;
  switch (st) {
    case omp_sched_auto:
      stringStream << "auto";
      break;
    case omp_sched_dynamic:
      stringStream << "dynamic";
      break;
    case omp_sched_guided:
      stringStream << "guided";
      break;
    case omp_sched_static:
      stringStream << "static";
      break;
    default:
      stringStream << "?";
      break;
  }
  stringStream << "_" << n << "_" << ss;
  std::cout << std::setw(50) << std::left << stringStream.str() << std::setw(12)
            << std::right << "" << std::setw(14) << std::fixed
            << std::setprecision(3) << dur.count() << " ms" << std::endl;
  for (int i = 0; i < omp_get_max_threads(); ++i) {
    std::ostringstream stringStream2;
    stringStream2 << "Thread"
                  << " " << i;
    std::ostringstream stringStream3;

    for (const auto v : result[i]) {
      stringStream3 << v << " ";
    }
    std::cout << "  " << std::setw(14) << std::left << stringStream2.str()
              << std::setw(34) << std::left << stringStream3.str().substr(0, 33)
              << std::setw(12) << std::right << iterations[i] << std::setw(14)
              << std::fixed << std::setprecision(3) << timing[i].count()
              << " ms" << std::endl;
  }
}

// ____________________________________________________________________________
int main() {
  std::vector<int> runs{117,    265,    567,    934,    1 << 4,
                        1 << 6, 1 << 8, 1 << 9, 1 << 10};
  std::sort(runs.begin(), runs.end());
  std::cout << "---------------------------------------------------------------"
               "----------------"
            << std::endl;
  std::cout << std::setw(50) << std::left << "Benchmark" << std::setw(12)
            << std::right << "Iterations" << std::setw(17) << std::right
            << "Time" << std::endl;
  std::cout << "---------------------------------------------------------------"
               "----------------"
            << std::endl;
  for (const auto n : runs) {
    run(n, omp_sched_static, 0);
    for (int i = 1; i < (n / omp_get_max_threads()); i *= 2) {
      run(n, omp_sched_static, i);
    }
  }
  for (const auto n : runs) {
    run(n, omp_sched_dynamic, 0);
    for (int i = 1; i < (n / omp_get_max_threads()); i *= 2) {
      run(n, omp_sched_dynamic, i);
    }
  }
  for (const auto n : runs) {
    run(n, omp_sched_guided, 0);
    for (int i = 1; i < (n / omp_get_max_threads()); i *= 2) {
      run(n, omp_sched_guided, i);
    }
  }
  return 0;
}