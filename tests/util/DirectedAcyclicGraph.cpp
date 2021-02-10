// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DirectedAcyclicGraph.h"

#include "gtest/gtest.h"
#include "osm2ttl/util/DirectedGraph.h"

namespace osm2ttl::util {

TEST(DirectedAcyclicGraph, empty) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  ASSERT_EQ(0, src.getNumVertices());
  ASSERT_EQ(0, src.getNumEdges());

  osm2ttl::util::DirectedGraph<uint8_t> res =
      osm2ttl::util::reduceDAG(src, false);
  ASSERT_EQ(0, res.getNumVertices());
  ASSERT_EQ(0, res.getNumEdges());
}

/*
Graph used for WhiteboardExample1

6   4    3
 \ / \   |
  2   1  |
  |   | /
  |   0
   \ /
    5
*/
osm2ttl::util::DirectedGraph<uint8_t> createWhiteboardExample1ExpectedResult() {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(1, 4);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 2);
  return src;
}

TEST(DirectedAcyclicGraph, WhiteboardExample1AllInAll) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
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
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(12, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample1AllInAll.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample1ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample1AllInAll.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample1AllInAll.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, WhiteboardExample1SingleThreaded) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(1, 4);
  src.addEdge(2, 4);
  src.addEdge(2, 6);
  src.addEdge(5, 0);
  src.addEdge(5, 2);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(7, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample1SingleThreaded.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample1ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample1SingleThreaded.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample1SingleThreaded.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, WhiteboardExample1MultiThreaded) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
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
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(11, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample1MultiThreaded.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample1ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample1MultiThreaded.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample1MultiThreaded.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

/*
Graph used for WhiteboardExample2

  6
 /|\
|  \ \
4   0 |
 \ /  |
  2   3
 / \ /
1   5
*/
osm2ttl::util::DirectedGraph<uint8_t> createWhiteboardExample2ExpectedResult() {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
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

TEST(DirectedAcyclicGraph, WhiteboardExample2AllConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
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
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(15, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample2AllConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample2ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample2AllConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample2AllConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, WhiteboardExample2MinimalConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 6);
  src.addEdge(1, 2);
  src.addEdge(2, 0);
  src.addEdge(2, 4);
  src.addEdge(3, 6);
  src.addEdge(4, 6);
  src.addEdge(5, 2);
  src.addEdge(5, 3);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(8, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample2MinimalConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample2ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample2MinimalConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample2MinimalConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, WhiteboardExample2) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
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
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(14, src.getNumEdges());

  // src.dump("/tmp/WhiteboardExample2.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createWhiteboardExample2ExpectedResult();
  // expected.dump("/tmp/WhiteboardExample2.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/WhiteboardExample2.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

// LineExample1
osm2ttl::util::DirectedGraph<uint8_t> createLineExample1ExpectedResult() {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(1, 2);
  src.addEdge(2, 3);
  src.addEdge(3, 4);
  src.addEdge(4, 5);
  src.addEdge(5, 6);
  return src;
}

TEST(DirectedAcyclicGraph, LineExample1AllConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 2);
  src.addEdge(0, 3);
  src.addEdge(0, 4);
  src.addEdge(0, 5);
  src.addEdge(0, 6);
  src.addEdge(1, 2);
  src.addEdge(1, 3);
  src.addEdge(1, 4);
  src.addEdge(1, 5);
  src.addEdge(1, 6);
  src.addEdge(2, 3);
  src.addEdge(2, 4);
  src.addEdge(2, 5);
  src.addEdge(2, 6);
  src.addEdge(3, 4);
  src.addEdge(3, 5);
  src.addEdge(3, 6);
  src.addEdge(4, 5);
  src.addEdge(4, 6);
  src.addEdge(5, 6);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(21, src.getNumEdges());

  {
    // src.dump("/tmp/LineExample1AllConnections.source.dot");
    const osm2ttl::util::DirectedGraph expected =
        createLineExample1ExpectedResult();
    // expected.dump("/tmp/LineExample1AllConnections.expected.dot");
    const osm2ttl::util::DirectedGraph result =
        osm2ttl::util::reduceDAG(src, false);
    // result.dump("/tmp/LineExample1AllConnections.result.dot");
    ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
    ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
    for (const auto& vertexId : expected.getVertices()) {
      const auto& expectedEdges = expected.getEdges(vertexId);
      const auto& resultEdges = result.getEdges(vertexId);
      ASSERT_EQ(expectedEdges.size(), resultEdges.size())
          << " error for vertex: " << vertexId;
      for (size_t i = 0; i < expectedEdges.size(); ++i) {
        ASSERT_EQ(expectedEdges[i], resultEdges[i])
            << " error for vertex: " << vertexId << " at entry " << i;
      }
    }
  }
  {
    // src.dump("/tmp/LineExample1AllConnections.source.dot");
    const osm2ttl::util::DirectedGraph expected =
        createLineExample1ExpectedResult();
    // expected.dump("/tmp/LineExample1AllConnections.expected.dot");
    const osm2ttl::util::DirectedGraph result =
        osm2ttl::util::reduceMaximalConnectedDAG(src, false);
    // result.dump("/tmp/LineExample1AllConnections.result.dot");
    ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
    ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
    for (const auto& vertexId : expected.getVertices()) {
      const auto& expectedEdges = expected.getEdges(vertexId);
      const auto& resultEdges = result.getEdges(vertexId);
      ASSERT_EQ(expectedEdges.size(), resultEdges.size())
          << " error for vertex: " << vertexId;
      for (size_t i = 0; i < expectedEdges.size(); ++i) {
        ASSERT_EQ(expectedEdges[i], resultEdges[i])
            << " error for vertex: " << vertexId << " at entry " << i;
      }
    }
  }
}

TEST(DirectedAcyclicGraph, LineExample1EvenIdAllConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 2);
  src.addEdge(0, 3);
  src.addEdge(0, 4);
  src.addEdge(0, 5);
  src.addEdge(0, 6);
  src.addEdge(1, 2);
  src.addEdge(2, 3);
  src.addEdge(2, 4);
  src.addEdge(2, 5);
  src.addEdge(2, 6);
  src.addEdge(3, 4);
  src.addEdge(4, 5);
  src.addEdge(4, 6);
  src.addEdge(5, 6);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(15, src.getNumEdges());

  // src.dump("/tmp/LineExample1EvenIdAllConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createLineExample1ExpectedResult();
  // expected.dump("/tmp/LineExample1EvenIdAllConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/LineExample1EvenIdAllConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, LineExample1OddIdAllConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(1, 2);
  src.addEdge(1, 3);
  src.addEdge(1, 4);
  src.addEdge(1, 5);
  src.addEdge(1, 6);
  src.addEdge(2, 3);
  src.addEdge(3, 4);
  src.addEdge(3, 5);
  src.addEdge(3, 6);
  src.addEdge(4, 5);
  src.addEdge(5, 6);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(12, src.getNumEdges());

  // src.dump("/tmp/LineExample1OddIdAllConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createLineExample1ExpectedResult();
  // expected.dump("/tmp/LineExample1OddIdAllConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/LineExample1OddIdAllConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, LineExample1IdZeroAllConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 2);
  src.addEdge(0, 3);
  src.addEdge(0, 4);
  src.addEdge(0, 5);
  src.addEdge(0, 6);
  src.addEdge(1, 2);
  src.addEdge(2, 3);
  src.addEdge(3, 4);
  src.addEdge(4, 5);
  src.addEdge(5, 6);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(11, src.getNumEdges());

  // src.dump("/tmp/LineExample1IdZeroAllConnections.source.dot");
  const osm2ttl::util::DirectedGraph expected =
      createLineExample1ExpectedResult();
  // expected.dump("/tmp/LineExample1IdZeroAllConnections.expected.dot");
  const osm2ttl::util::DirectedGraph result =
      osm2ttl::util::reduceDAG(src, false);
  // result.dump("/tmp/LineExample1IdZeroAllConnections.result.dot");
  ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
  ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
  for (const auto& vertexId : expected.getVertices()) {
    const auto& expectedEdges = expected.getEdges(vertexId);
    const auto& resultEdges = result.getEdges(vertexId);
    ASSERT_EQ(expectedEdges.size(), resultEdges.size())
        << " error for vertex: " << vertexId;
    for (size_t i = 0; i < expectedEdges.size(); ++i) {
      ASSERT_EQ(expectedEdges[i], resultEdges[i])
          << " error for vertex: " << vertexId << " at entry " << i;
    }
  }
}

TEST(DirectedAcyclicGraph, LineExample1IdZeroOnlyOddConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 3);
  src.addEdge(0, 5);
  src.addEdge(1, 2);
  src.addEdge(2, 3);
  src.addEdge(3, 4);
  src.addEdge(4, 5);
  src.addEdge(5, 6);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(8, src.getNumEdges());

  {
    // src.dump("/tmp/LineExample1IdZeroOnlyOddConnections.source.dot");
    const osm2ttl::util::DirectedGraph expected =
        createLineExample1ExpectedResult();
    // expected.dump("/tmp/LineExample1IdZeroOnlyOddConnections.expected.dot");
    const osm2ttl::util::DirectedGraph result =
        osm2ttl::util::reduceDAG(src, false);
    // result.dump("/tmp/LineExample1IdZeroOnlyOddConnections.result.dot");
    ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
    ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
    for (const auto& vertexId : expected.getVertices()) {
      const auto& expectedEdges = expected.getEdges(vertexId);
      const auto& resultEdges = result.getEdges(vertexId);
      ASSERT_EQ(expectedEdges.size(), resultEdges.size())
          << " error for vertex: " << vertexId;
      for (size_t i = 0; i < expectedEdges.size(); ++i) {
        ASSERT_EQ(expectedEdges[i], resultEdges[i])
            << " error for vertex: " << vertexId << " at entry " << i;
      }
    }
  }
  {
    // Check that reduceMaximalConnectedDAG does produce correct result as each
    // node knows the one missing. This would not work if more then one node
    // would be skipped... see different test.
    // src.dump("/tmp/LineExample1IdZeroOnlyOddConnections.source.dot");
    const osm2ttl::util::DirectedGraph expected =
        createLineExample1ExpectedResult();
    // expected.dump("/tmp/LineExample1IdZeroOnlyOddConnections.expected.dot");
    const osm2ttl::util::DirectedGraph result =
        osm2ttl::util::reduceMaximalConnectedDAG(src, false);
    // result.dump("/tmp/LineExample1IdZeroOnlyOddConnections.result.dot");
    ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
    // some connections are missing - this is expected
    ASSERT_NE(expected.getNumEdges(), result.getNumEdges());
    ASSERT_NE(expected.getEdges(0).size(), result.getEdges(0).size());
    ASSERT_EQ(expected.getEdges(1).size(), result.getEdges(1).size());
    ASSERT_EQ(expected.getEdges(2).size(), result.getEdges(2).size());
    ASSERT_EQ(expected.getEdges(3).size(), result.getEdges(3).size());
    ASSERT_EQ(expected.getEdges(4).size(), result.getEdges(4).size());
    ASSERT_EQ(expected.getEdges(5).size(), result.getEdges(5).size());
    ASSERT_EQ(expected.getEdges(6).size(), result.getEdges(6).size());
  }
}

TEST(DirectedAcyclicGraph, LineExample1IdZeroOnlyOneAdditionalConnections) {
  osm2ttl::util::DirectedGraph<uint8_t> src{};
  src.addEdge(0, 1);
  src.addEdge(0, 4);
  src.addEdge(1, 2);
  src.addEdge(2, 3);
  src.addEdge(3, 4);
  src.addEdge(4, 5);
  src.addEdge(5, 6);
  src.prepareFindSuccessorsFast();
  ASSERT_EQ(7, src.getNumVertices());
  ASSERT_EQ(7, src.getNumEdges());

  {
    // src.dump("/tmp/LineExample1IdZeroOnlyOddConnections.source.dot");
    const osm2ttl::util::DirectedGraph expected =
        createLineExample1ExpectedResult();
    // expected.dump("/tmp/LineExample1IdZeroOnlyOddConnections.expected.dot");
    const osm2ttl::util::DirectedGraph result =
        osm2ttl::util::reduceDAG(src, false);
    // result.dump("/tmp/LineExample1IdZeroOnlyOddConnections.result.dot");
    ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
    ASSERT_EQ(expected.getNumEdges(), result.getNumEdges());
    for (const auto& vertexId : expected.getVertices()) {
      const auto& expectedEdges = expected.getEdges(vertexId);
      const auto& resultEdges = result.getEdges(vertexId);
      ASSERT_EQ(expectedEdges.size(), resultEdges.size())
          << " error for vertex: " << vertexId;
      for (size_t i = 0; i < expectedEdges.size(); ++i) {
        ASSERT_EQ(expectedEdges[i], resultEdges[i])
            << " error for vertex: " << vertexId << " at entry " << i;
      }
    }
  }
  {
    // Check that reduceMaximalConnectedDAG does produce correct result as each
    // node knows the one missing. This would not work if more then one node
    // would be skipped... see different test.
    // src.dump("/tmp/LineExample1IdZeroOnlyOddConnections.source.dot");
    const osm2ttl::util::DirectedGraph expected =
        createLineExample1ExpectedResult();
    // expected.dump("/tmp/LineExample1IdZeroOnlyOddConnections.expected.dot");
    const osm2ttl::util::DirectedGraph result =
        osm2ttl::util::reduceMaximalConnectedDAG(src, false);
    // result.dump("/tmp/LineExample1IdZeroOnlyOddConnections.result.dot");
    ASSERT_EQ(expected.getNumVertices(), result.getNumVertices());
    // some connections are missing - this is expected
    ASSERT_NE(expected.getNumEdges(), result.getNumEdges());
    ASSERT_NE(expected.getEdges(0).size(), result.getEdges(0).size());
    ASSERT_EQ(expected.getEdges(1).size(), result.getEdges(1).size());
    ASSERT_EQ(expected.getEdges(2).size(), result.getEdges(2).size());
    ASSERT_EQ(expected.getEdges(3).size(), result.getEdges(3).size());
    ASSERT_EQ(expected.getEdges(4).size(), result.getEdges(4).size());
    ASSERT_EQ(expected.getEdges(5).size(), result.getEdges(5).size());
    ASSERT_EQ(expected.getEdges(6).size(), result.getEdges(6).size());
  }
}

}  // namespace osm2ttl::util