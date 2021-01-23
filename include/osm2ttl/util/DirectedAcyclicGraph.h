// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_DIRECTEDACYLICGRAPH_H
#define OSM2TTL_UTIL_DIRECTEDACYLICGRAPH_H


#include <algorithm>

#include "DirectedGraph.h"
#include "ProgressBar.h"

namespace osm2ttl {
namespace util {

// reduceDAG returns a reduced DAG from a given sorted DAG
template <typename T>
osm2ttl::util::DirectedGraph<T> reduceDAG(
    const osm2ttl::util::DirectedGraph<T>& sourceDAG, bool showProgress) {
  osm2ttl::util::DirectedGraph<T> result;
  osm2ttl::util::ProgressBar progressBar{sourceDAG.getNumVertices(), showProgress};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  // Reduce each adjacency list
  const auto& vertices = sourceDAG.getVertices();
#pragma omp parallel for shared(vertices, sourceDAG, result, progressBar, \
                                entryCount) default(none)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto& src = vertices[i];
    std::vector<T> possibleEdges(sourceDAG.getEdgesFast(src));
    std::vector<T> edges;
    for (const auto& dst : sourceDAG.getEdgesFast(src)) {
      const auto& dstEdges = sourceDAG.findSuccessorsFast(dst);
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
  osm2ttl::util::ProgressBar progressBar{sourceDAG.getNumVertices(), showProgress};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  // Reduce each adjacency list
  const auto& vertices = sourceDAG.getVertices();
#pragma omp parallel for shared(vertices, sourceDAG, result, progressBar, \
                                entryCount) default(none)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto& src = vertices[i];
    std::vector<T> possibleEdges(sourceDAG.getEdges(src));
    std::vector<T> edges;
    for (const auto& dst : sourceDAG.getEdges(src)) {
      const auto& dstEdges = sourceDAG.getEdges(dst);
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
