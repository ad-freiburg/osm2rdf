// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "gtest/gtest.h"

#include "osm2ttl/util/DirectedGraph.h"

TEST(DirectedGraph, findAbove) {
  osm2ttl::util::DirectedGraph g;
  {
    auto res = g.findAbove(1);
    ASSERT_EQ(0, res.size());
  }
  {
    g.addEdge(1, 0);
    auto res = g.findAbove(1);
    ASSERT_EQ(1, res.size());
    ASSERT_EQ(0, res[0]);
  }
  {
    g.addEdge(1, 2);
    auto res = g.findAbove(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
}