// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DirectedGraph.h"

#include "gtest/gtest.h"

osm2ttl::util::DirectedGraph createEmptyDirectedGraph() {
  return osm2ttl::util::DirectedGraph{};
}

TEST(DirectedGraph, addEdge) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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

TEST(DirectedGraph, findSuccessors) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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

TEST(DirectedGraph, findSuccessorsFast) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
  g.addEdge(1, 2);
  g.addEdge(1, 0);
  g.addEdge(1, 2);
  {
    ASSERT_ANY_THROW(g.findSuccessorsFast(1));
  }
  {
    g.prepareFindSuccessorsFast();
    const auto res = g.findSuccessorsFast(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
}

TEST(DirectedGraph, sort) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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

TEST(DirectedGraph, prepareFindSuccessorsFast) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
  g.addEdge(1, 2);
  g.addEdge(1, 0);
  g.addEdge(1, 2);
  {
    ASSERT_ANY_THROW(g.findSuccessorsFast(1));
  }
  {
    g.prepareFindSuccessorsFast();
    const auto res = g.findSuccessorsFast(1);
    ASSERT_EQ(2, res.size());
    ASSERT_EQ(0, res[0]);
    ASSERT_EQ(2, res[1]);
  }
}

TEST(DirectedGraph, getNumEdges) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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

TEST(DirectedGraph, getNumVertices) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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

TEST(DirectedGraph, getVertices) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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

TEST(DirectedGraph, getEdges) {
  osm2ttl::util::DirectedGraph g = createEmptyDirectedGraph();
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