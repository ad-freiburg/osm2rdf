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

#include "osm2ttl/util/DirectedGraph.h"

#include "gtest/gtest.h"

namespace osm2ttl::util {

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, Constructor) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  ASSERT_EQ(0, g.getNumVertices());
  ASSERT_EQ(0, g.getNumEdges());
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, CopyConstructor) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  g.addEdge(1, 2);
  g.addEdge(1, 2);
  g.addEdge(2, 1);
  ASSERT_EQ(2, g.getNumVertices());
  ASSERT_EQ(3, g.getNumEdges());
  osm2ttl::util::DirectedGraph g2{g};
  ASSERT_EQ(2, g2.getNumVertices());
  ASSERT_EQ(3, g2.getNumEdges());
  g2.addEdge(2, 3);
  ASSERT_EQ(2, g.getNumVertices());
  ASSERT_EQ(3, g.getNumEdges());
  ASSERT_EQ(3, g2.getNumVertices());
  ASSERT_EQ(4, g2.getNumEdges());
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, addEdge) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  ASSERT_EQ(0, g.getNumVertices());
  ASSERT_EQ(0, g.getNumEdges());
  // Allows multiple edges between vertices
  g.addEdge(1, 2);
  ASSERT_EQ(2, g.getNumVertices());
  ASSERT_EQ(1, g.getNumEdges());
  g.addEdge(1, 2);
  ASSERT_EQ(2, g.getNumVertices());
  ASSERT_EQ(2, g.getNumEdges());
  // Allows creation of cycles
  g.addEdge(2, 1);
  ASSERT_EQ(2, g.getNumVertices());
  ASSERT_EQ(3, g.getNumEdges());
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, findSuccessors) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  {
    auto res = g.findSuccessors(1);
    ASSERT_EQ(0, res.size());
  }
  {
    g.addEdge(1, 2);
    auto res = g.findSuccessors(1);
    ASSERT_EQ(1, res.size());
    ASSERT_EQ(2, res[0]);
  }
  {
    g.addEdge(1, 0);
    auto res = g.findSuccessors(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
  {
    g.addEdge(1, 2);
    auto res = g.findSuccessors(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, findSuccessorsFast) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  g.addEdge(1, 2);
  g.addEdge(1, 0);
  g.addEdge(1, 2);
  { ASSERT_ANY_THROW(g.findSuccessorsFast(1)); }
  g.prepareFindSuccessorsFast();
  {
    const auto res = g.findSuccessorsFast(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
  {
    const auto res = g.findSuccessorsFast(4);
    ASSERT_EQ(0, res.size());
  }
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, sort) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  {
    g.addEdge(1, 2);
    g.addEdge(1, 0);
    g.addEdge(1, 2);
  }
  {
    auto res = g.getEdges(1);
    ASSERT_EQ(3, res.size());
    ASSERT_EQ(2, res[0]);
    ASSERT_EQ(0, res[1]);
    ASSERT_EQ(2, res[2]);
  }
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, prepareFindSuccessorsFast) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  g.addEdge(1, 2);
  g.addEdge(1, 0);
  g.addEdge(1, 2);
  { ASSERT_ANY_THROW(g.findSuccessorsFast(1)); }
  {
    g.prepareFindSuccessorsFast();
    const auto res = g.findSuccessorsFast(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, getNumEdges) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  ASSERT_EQ(0, g.getNumEdges());
  // Count each edge, allows multiple edges between the same vertices.
  g.addEdge(1, 2);
  ASSERT_EQ(1, g.getNumEdges());
  g.addEdge(1, 2);
  ASSERT_EQ(2, g.getNumEdges());
  g.addEdge(1, 2);
  ASSERT_EQ(3, g.getNumEdges());
  g.addEdge(3, 4);
  ASSERT_EQ(4, g.getNumEdges());
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, getNumVertices) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  ASSERT_EQ(0, g.getNumVertices());
  // Counts each vertex only once...
  g.addEdge(1, 1);
  ASSERT_EQ(1, g.getNumVertices());
  g.addEdge(1, 1);
  ASSERT_EQ(1, g.getNumVertices());
  // ... but counts all unique vertices.
  g.addEdge(1, 2);
  ASSERT_EQ(2, g.getNumVertices());
  g.addEdge(3, 4);
  ASSERT_EQ(4, g.getNumVertices());
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, getVertices) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  {
    auto res = g.getVertices();
    ASSERT_EQ(0, res.size());
  }
  {
    g.addEdge(1, 1);
    auto res = g.getVertices();
    ASSERT_EQ(1, res.size());
    ASSERT_EQ(1, res[0]);
  }
  {
    g.addEdge(3, 1);
    auto res = g.getVertices();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(3, res[1]);
  }
  {
    g.addEdge(1, 2);
    auto res = g.getVertices();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(3, res.size());
    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(2, res[1]);
    ASSERT_EQ(3, res[2]);
  }
}

// ____________________________________________________________________________
TEST(UTIL_DirectedGraph, getEdges) {
  osm2ttl::util::DirectedGraph<uint8_t> g{};
  {
    g.addEdge(1, 1);
    const auto res = g.getEdges(1);
    ASSERT_EQ(1, res.size());
    ASSERT_EQ(1, res[0]);
  }
  {
    g.addEdge(3, 1);
    const auto res = g.getEdges(1);
    ASSERT_EQ(1, res.size());
    ASSERT_EQ(1, res[0]);
  }
  {
    g.addEdge(1, 3);
    const auto res = g.getEdges(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(3, res[1]);
  }
  {
    g.addEdge(1, 2);
    const auto res = g.getEdges(1);
    ASSERT_EQ(3, res.size());
    ASSERT_EQ(1, res[0]);
    ASSERT_EQ(3, res[1]);
    ASSERT_EQ(2, res[2]);
  }
}

}  // namespace osm2ttl::util