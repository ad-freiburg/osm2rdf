// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "gtest/gtest.h"

namespace osm2ttl {
// Misc tests to ensure (external) library functions work as expected

// std::set_difference
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

}