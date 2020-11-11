// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "gtest/gtest.h"


#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/GeometryHandler.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/DirectedGraph.h"

TEST(GeometryHandlerReduceDAG, empty) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  ASSERT_EQ(0, src.getNumVertices());
  ASSERT_EQ(0, src.getNumEdges());

  osm2ttl::util::DirectedGraph res = geometryHandler.reduceDAG(src, false);
  ASSERT_EQ(0, res.getNumVertices());
  ASSERT_EQ(0, res.getNumEdges());
}

TEST(GeometryHandlerReduceDAG, WhiteboardExampleAllInAll) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(0, 4);
  src.addEdge(1, 4);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 1);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  src.addEdge(5, 4);
  src.addEdge(5, 6);
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(12, src.getNumEdges());

  osm2ttl::util::DirectedGraph res = geometryHandler.reduceDAG(src, false);
  ASSERT_EQ(7, res.getNumVertices());
  ASSERT_EQ(7, res.getNumEdges());
  {
    auto edges = res.getEdges(0);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(1, edges[0]);
    ASSERT_EQ(3, edges[1]);
  }
  {
    auto edges = res.getEdges(1);
    ASSERT_EQ(1, edges.size());
    ASSERT_EQ(4, edges[0]);
  }
  {
    auto edges = res.getEdges(2);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(4, edges[0]);
    ASSERT_EQ(6, edges[1]);
  }
  {
    auto edges = res.getEdges(3);
    ASSERT_EQ(0, edges.size());
  }
  {
    auto edges = res.getEdges(4);
    ASSERT_EQ(0, edges.size());
  }
  {
    auto edges = res.getEdges(5);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(0, edges[0]);
    ASSERT_EQ(2, edges[1]);
  }
  {
    auto edges = res.getEdges(6);
    ASSERT_EQ(0, edges.size());
  }
}

TEST(GeometryHandlerReduceDAG, WhiteboardExampleSingleThreaded) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(1, 4);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 2);
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(7, src.getNumEdges());

  osm2ttl::util::DirectedGraph res = geometryHandler.reduceDAG(src, false);
  ASSERT_EQ(7, res.getNumVertices());
  ASSERT_EQ(7, res.getNumEdges());
  {
    auto edges = res.getEdges(0);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(1, edges[0]);
    ASSERT_EQ(3, edges[1]);
  }
  {
    auto edges = res.getEdges(1);
    ASSERT_EQ(1, edges.size());
    ASSERT_EQ(4, edges[0]);
  }
  {
    auto edges = res.getEdges(2);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(4, edges[0]);
    ASSERT_EQ(6, edges[1]);
  }
  {
    auto edges = res.getEdges(3);
    ASSERT_EQ(0, edges.size());
  }
  {
    auto edges = res.getEdges(4);
    ASSERT_EQ(0, edges.size());
  }
  {
    auto edges = res.getEdges(5);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(0, edges[0]);
    ASSERT_EQ(2, edges[1]);
  }
  {
    auto edges = res.getEdges(6);
    ASSERT_EQ(0, edges.size());
  }
}

TEST(GeometryHandlerReduceDAG, WhiteboardExampleMultiThreaded) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(0, 4);
  src.addEdge(1, 4);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 1);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  src.addEdge(5, 4);
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(11, src.getNumEdges());

  osm2ttl::util::DirectedGraph res = geometryHandler.reduceDAG(src, false);
  ASSERT_EQ(7, res.getNumVertices());
  ASSERT_EQ(7, res.getNumEdges());
  {
    auto edges = res.getEdges(0);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(1, edges[0]);
    ASSERT_EQ(3, edges[1]);
  }
  {
    auto edges = res.getEdges(1);
    ASSERT_EQ(1, edges.size());
    ASSERT_EQ(4, edges[0]);
  }
  {
    auto edges = res.getEdges(2);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(4, edges[0]);
    ASSERT_EQ(6, edges[1]);
  }
  {
    auto edges = res.getEdges(3);
    ASSERT_EQ(0, edges.size());
  }
  {
    auto edges = res.getEdges(4);
    ASSERT_EQ(0, edges.size());
  }
  {
    auto edges = res.getEdges(5);
    ASSERT_EQ(2, edges.size());
    ASSERT_EQ(0, edges[0]);
    ASSERT_EQ(2, edges[1]);
  }
  {
    auto edges = res.getEdges(6);
    ASSERT_EQ(0, edges.size());
  }
}