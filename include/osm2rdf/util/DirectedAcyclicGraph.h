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

#ifndef OSM2RDF_UTIL_DIRECTEDACYLICGRAPH_H
#define OSM2RDF_UTIL_DIRECTEDACYLICGRAPH_H

#include <algorithm>

#include "DirectedGraph.h"
#include "ProgressBar.h"

namespace osm2rdf::util {

// reduceDAG returns a reduced DAG from a given sorted DAG
template <typename T>
osm2rdf::util::DirectedGraph<T> reduceDAG(
    const osm2rdf::util::DirectedGraph<T>& sourceDAG, bool showProgress) {
  osm2rdf::util::DirectedGraph<T> result;
  osm2rdf::util::ProgressBar progressBar{sourceDAG.getNumVertices(),
                                         showProgress};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  // Reduce each adjacency list
  const auto& vertices = sourceDAG.getVertices();
#pragma omp parallel for shared(vertices, sourceDAG, result, progressBar, \
                                entryCount) default(none)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto& src = vertices[i];
    std::vector<T> possibleEdges(sourceDAG.findSuccessors(src));
    std::vector<T> edges;
    for (const auto& dst : sourceDAG.findSuccessors(src)) {
      const auto& dstEdges = sourceDAG.findSuccessors(dst);
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
#pragma omp critical
    progressBar.update(entryCount++);
  }
  progressBar.done();
  return result;
}

// reduceMaximalConnectedDAG returns a reduced DAG from a given sorted and
// maximal connected DAG
template <typename T>
osm2rdf::util::DirectedGraph<T> reduceMaximalConnectedDAG(
    const osm2rdf::util::DirectedGraph<T>& sourceDAG, bool showProgress) {
  osm2rdf::util::DirectedGraph<T> result;
  osm2rdf::util::ProgressBar progressBar{sourceDAG.getNumVertices(),
                                         showProgress};
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
#pragma omp critical
    progressBar.update(entryCount++);
  }
  progressBar.done();
  return result;
}

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_DIRECTEDACYLICGRAPH_H
