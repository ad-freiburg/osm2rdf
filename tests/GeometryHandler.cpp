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

// Graph used for WhiteboardExample1
//
// 6   4   3
//  \ / \ /
//   2   0
//   |   |
//   |   1
//    \ /
//     5
//
osm2ttl::util::DirectedGraph createWhiteboardExample1ExpectedResult() {
  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(1, 4);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 2);
  return src;
}

TEST(GeometryHandlerReduceDAG, WhiteboardExample1AllInAll) {
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

  // src.dump("/tmp/WhiteboardExample1AllInAll.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample1ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample1AllInAll.expected.dot");
  const osm2ttl::util::DirectedGraph result = geometryHandler.reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample1AllInAll.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), expectedEdges.size());
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], expectedEdges[i]) << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(GeometryHandlerReduceDAG, WhiteboardExample1SingleThreaded) {
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

  // src.dump("/tmp/WhiteboardExample1SingleThreaded.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample1ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample1SingleThreaded.expected.dot");
  const osm2ttl::util::DirectedGraph result = geometryHandler.reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample1SingleThreaded.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), expectedEdges.size());
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], expectedEdges[i]) << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(GeometryHandlerReduceDAG, WhiteboardExample1MultiThreaded) {
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

  // src.dump("/tmp/WhiteboardExample1MultiThreaded.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample1ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample1MultiThreaded.expected.dot");
  const osm2ttl::util::DirectedGraph result = geometryHandler.reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample1MultiThreaded.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), expectedEdges.size());
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], expectedEdges[i]) << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

// Graph used for WhiteboardExample2
//
//     6
//    /|\
//   |  \ \
//   4   0 |
//    \ /  |
//     2   3
//    / \ /
//   1   5
//
osm2ttl::util::DirectedGraph createWhiteboardExample2ExpectedResult() {
  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 6);
  src.addEdge(1, 2);
  src.addEdge(2, 0);
  src.addEdge(2, 4);
  src.addEdge(3, 6);
  src.addEdge(4, 6);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  return src;
}

TEST(GeometryHandlerReduceDAG, WhiteboardExample2AllConnections) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 6);
  src.addEdge(1, 0);
  src.addEdge(1, 2);
  src.addEdge(1, 4);
  src.addEdge(1, 6);
  src.addEdge(2, 0);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(3, 6);
  src.addEdge(4, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  src.addEdge(5, 4);
  src.addEdge(5, 6);
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(15, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample2AllConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample2ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample2AllConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result = geometryHandler.reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample2AllConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), expectedEdges.size());
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], expectedEdges[i]) << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(GeometryHandlerReduceDAG, WhiteboardExample2MinimalConnections) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 6);
  src.addEdge(1, 2);
  src.addEdge(2, 0);
  src.addEdge(2, 4);
  src.addEdge(3, 6);
  src.addEdge(4, 6);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(8, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample2MinimalConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample2ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample2MinimalConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result = geometryHandler.reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample2MinimalConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), expectedEdges.size());
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], expectedEdges[i]) << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(GeometryHandlerReduceDAG, WhiteboardExample2) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config};
  osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT> geometryHandler{config, &w};

  osm2ttl::util::DirectedGraph src{};
  src.addEdge(0, 6);
  src.addEdge(1, 0);
  src.addEdge(1, 2);
  src.addEdge(1, 4);
  src.addEdge(1, 6);
  src.addEdge(2, 0);
  src.addEdge(2, 4);
  src.addEdge(3, 6);
  src.addEdge(4, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  src.addEdge(5, 4);
  src.addEdge(5, 6);
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(14, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample2.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample2ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample2.expected.dot");
  const osm2ttl::util::DirectedGraph result = geometryHandler.reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample2.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), expectedEdges.size());
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], expectedEdges[i]) << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}