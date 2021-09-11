// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#include "gtest/gtest.h"

namespace osm2rdf {
// Misc tests to ensure (external) library functions work as expected

// ____________________________________________________________________________
TEST(Baseline, set_difference) {
  {
    std::vector<int> i1{1, 2, 3, 4, 3, 2, 1, 1, 1};
    std::vector<int> i2{1, 3};
    std::vector<int> result;

    std::sort(i1.begin(), i1.end());
    const auto it1 = std::unique(i1.begin(), i1.end());
    i1.resize(std::distance(i1.begin(), it1));

    std::sort(i2.begin(), i2.end());
    const auto it2 = std::unique(i2.begin(), i2.end());
    i2.resize(std::distance(i2.begin(), it2));

    std::set_difference(i1.begin(), i1.end(), i1.begin(), i1.end(),
                        std::back_inserter(result));

    ASSERT_EQ(0, result.size());
  }
  {
    std::vector<int> i1{1, 2, 3, 4, 3, 2, 1, 1, 1};
    std::vector<int> i2{1, 3};
    std::vector<int> result;

    std::sort(i1.begin(), i1.end());
    const auto it1 = std::unique(i1.begin(), i1.end());
    i1.resize(std::distance(i1.begin(), it1));

    std::sort(i2.begin(), i2.end());
    const auto it2 = std::unique(i2.begin(), i2.end());
    i2.resize(std::distance(i2.begin(), it2));

    std::set_difference(i2.begin(), i2.end(), i2.begin(), i2.end(),
                        std::back_inserter(result));

    ASSERT_EQ(0, result.size());
  }
  {
    std::vector<int> i1{1, 2, 3, 4, 3, 2, 1, 1, 1};
    std::vector<int> i2{1, 3};
    std::vector<int> result;

    std::sort(i1.begin(), i1.end());
    const auto it1 = std::unique(i1.begin(), i1.end());
    i1.resize(std::distance(i1.begin(), it1));

    std::sort(i2.begin(), i2.end());
    const auto it2 = std::unique(i2.begin(), i2.end());
    i2.resize(std::distance(i2.begin(), it2));

    std::set_difference(i1.begin(), i1.end(), i2.begin(), i2.end(),
                        std::back_inserter(result));
    ASSERT_EQ(2, result.size());
    ASSERT_EQ(2, result[0]);
    ASSERT_EQ(4, result[1]);
  }
  {
    std::vector<int> i1{1, 2, 3, 4, 3, 2, 1, 1, 1};
    std::vector<int> i2{1, 3};
    std::vector<int> result;
    std::sort(i1.begin(), i1.end());
    const auto it1 = std::unique(i1.begin(), i1.end());
    i1.resize(std::distance(i1.begin(), it1));

    std::sort(i2.begin(), i2.end());
    const auto it2 = std::unique(i2.begin(), i2.end());
    i2.resize(std::distance(i2.begin(), it2));

    std::set_difference(i2.begin(), i2.end(), i1.begin(), i1.end(),
                        std::back_inserter(result));

    ASSERT_EQ(0, result.size());
  }
}

// ____________________________________________________________________________
TEST(Baseline, stdoutRedirectionCOUT) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  std::cout << "Lorem ipsum";
  ASSERT_EQ("Lorem ipsum", buffer.str());

  // Restore std::cout
  std::cout.rdbuf(sbuf);
}

}  // namespace osm2rdf