// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_DIRECTEDACYLICGRAPH_H
#define OSM2TTL_UTIL_DIRECTEDACYLICGRAPH_H

#include <stdlib.h>
#include <algorithm>

#include "osmium/util/progress_bar.hpp"
#include "DirectedGraph.h"

namespace osm2ttl {
namespace util {

// reduceDAG returns a reduced DAG from a given sorted DAG
template <typename T>
osm2ttl::util::DirectedGraph<T> reduceDAG(
    const osm2ttl::util::DirectedGraph<T>& sourceDAG, bool showProgress) {
  osm2ttl::util::DirectedGraph<T> result;
  osmium::ProgressBar progressBar{sourceDAG.getNumVertices(), showProgress};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  // Reduce each adjacency list
  const auto& vertices = sourceDAG.getVertices();
#pragma omp parallel for shared(vertices, sourceDAG, result, progressBar, \
                                entryCount) default(none)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto& src = vertices[i];
    std::vector<T> possibleEdges(sourceDAG.getEdgesFast(src));
#ifndef NDEBUG
    auto sortedPossibleEdges = possibleEdges;
    std::sort(sortedPossibleEdges.begin(), sortedPossibleEdges.end());
    const auto& it = std::unique(sortedPossibleEdges.begin(), sortedPossibleEdges.end());
    sortedPossibleEdges.resize(std::distance(sortedPossibleEdges.begin(), it));
    assert(sortedPossibleEdges.size() == possibleEdges.size());
    for (size_t verifyI = 0; verifyI < possibleEdges.size(); ++verifyI) {
      assert(sortedPossibleEdges[verifyI] == possibleEdges[verifyI]);
    }
#endif
    std::vector<T> edges;
    for (const auto& dst : sourceDAG.getEdgesFast(src)) {
      const auto& dstEdges = sourceDAG.findSuccessorsFast(dst);
#ifndef NDEBUG
      auto sortedEdges = dstEdges;
      std::sort(sortedEdges.begin(), sortedEdges.end());
      const auto& it = std::unique(sortedEdges.begin(), sortedEdges.end());
      sortedEdges.resize(std::distance(sortedEdges.begin(), it));
      assert(sortedEdges.size() == dstEdges.size());
      for (size_t verifyI = 0; verifyI < dstEdges.size(); ++verifyI) {
        assert(sortedEdges[verifyI] == dstEdges[verifyI]);
      }
#endif
      std::set_difference(possibleEdges.begin(), possibleEdges.end(),
                          dstEdges.begin(), dstEdges.end(),
                          std::back_inserter(edges));
      possibleEdges = edges;
      edges.clear();
    }
#pragma omp critical(addEdge)
    {
      for (const auto& dst : possibleEdges) {
        result.addEdge(src, dst);
      }
    }
#pragma omp critical(progress)
    progressBar.update(entryCount++);
  }
  progressBar.done();
  return result;
}

// reduceMaximalConnectedDAG returns a reduced DAG from a given sorted and
// maximal connected DAG
template <typename T>
osm2ttl::util::DirectedGraph<T> reduceMaximalConnectedDAG(
    const osm2ttl::util::DirectedGraph<T>& sourceDAG, bool showProgress) {
  osm2ttl::util::DirectedGraph<T> result;
  osmium::ProgressBar progressBar{sourceDAG.getNumVertices(), showProgress};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  // Reduce each adjacency list
  const auto& vertices = sourceDAG.getVertices();
#pragma omp parallel for shared(vertices, sourceDAG, result, progressBar, \
                                entryCount) default(none)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto& src = vertices[i];
    std::vector<T> possibleEdges(sourceDAG.getEdgesFast(src));
#ifndef NDEBUG
    auto sortedPossibleEdges = possibleEdges;
    std::sort(sortedPossibleEdges.begin(), sortedPossibleEdges.end());
    const auto& it = std::unique(sortedPossibleEdges.begin(), sortedPossibleEdges.end());
    sortedPossibleEdges.resize(std::distance(sortedPossibleEdges.begin(), it));
    assert(sortedPossibleEdges.size() == possibleEdges.size());
    for (size_t verifyI = 0; verifyI < possibleEdges.size(); ++verifyI) {
      assert(sortedPossibleEdges[verifyI] == possibleEdges[verifyI]);
    }
#endif
    std::vector<T> edges;
    for (const auto& dst : sourceDAG.getEdgesFast(src)) {
      const auto& dstEdges = sourceDAG.getEdgesFast(dst);
#ifndef NDEBUG
      auto sortedEdges = dstEdges;
      std::sort(sortedEdges.begin(), sortedEdges.end());
      const auto& it = std::unique(sortedEdges.begin(), sortedEdges.end());
      sortedEdges.resize(std::distance(sortedEdges.begin(), it));
      assert(sortedEdges.size() == dstEdges.size());
      for (size_t verifyI = 0; verifyI < dstEdges.size(); ++verifyI) {
        assert(sortedEdges[verifyI] == dstEdges[verifyI]);
      }
#endif
      std::set_difference(possibleEdges.begin(), possibleEdges.end(),
                          dstEdges.begin(), dstEdges.end(),
                          std::back_inserter(edges));
      possibleEdges = edges;
      edges.clear();
    }
#pragma omp critical(addEdge)
    {
      for (const auto& dst : possibleEdges) {
        result.addEdge(src, dst);
      }
    }
#pragma omp critical(progress)
    progressBar.update(entryCount++);
  }
  progressBar.done();
  return result;
}

}
}

#endif  // OSM2TTL_UTIL_DIRECTEDACYLICGRAPH_H
