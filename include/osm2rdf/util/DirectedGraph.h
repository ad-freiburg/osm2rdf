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

#ifndef OSM2RDF_UTIL_DIRECTEDGRAPH_H
#define OSM2RDF_UTIL_DIRECTEDGRAPH_H

#include <stdint.h>

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace osm2rdf::util {

template <typename T>
class DirectedGraph {
 public:
  typedef T entry_t;
  // addEdge adds an edge between src and dst vertices. Allows multiple edges
  // between the same vertices.
  void addEdge(T src, T dst);
  // findSuccessors returns the ids of all successor vertices of the given
  // vertex.
  [[nodiscard]] std::vector<T> findSuccessors(T src) const;
  // findSuccessorsFast returns the same result as findSuccessors but faster,
  // after data is prepared for faster lookup.
  [[nodiscard]] std::vector<T> findSuccessorsFast(T src) const;
  // dump stores the complete graph in DOT-Format in a file at the given path.
  void dump(const std::filesystem::path& filename) const;
  // dumpOsm stores the complete graph in DOT-Format in a file at the given
  // path. This variant handles osm ids and converts for later lookup.
  void dumpOsm(const std::filesystem::path& filename) const;
  // prepareFindSuccessorsFast calculates for each vertex the successors and
  // stores them in a helper map.
  void prepareFindSuccessorsFast();
  // prepareFindSuccessorsFastNoLeafes calculates for each non leaf vertex the
  // successors and stores them in a helper map.
  void prepareFindSuccessorsFastNoLeafes();
  // getNumEdges returns the number of stored edges. If an edge is defined
  // multiple times it is counted multiple times too.
  [[nodiscard]] size_t getNumEdges() const;
  // getNumVertices returns the number of unique vertices in the graph.
  [[nodiscard]] size_t getNumVertices() const;
  // getVertices returns all unique vertices in the graph.
  [[nodiscard]] std::vector<T> getVertices() const;
  // getEdges returns the stored edges for the given vertex.
  [[nodiscard]] std::vector<T> getEdges(T src) const;
  // getEdges returns the stored edges but uses "fast" storage for lookup.
  [[nodiscard]] std::vector<T> getEdgesFast(T src) const;

 protected:
  void findSuccessorsHelper(T src, std::vector<T>* tmp) const;
  std::unordered_map<T, std::vector<T>> _adjacency;
  std::unordered_map<T, std::vector<T>> _successors;
  size_t _numEdges = 0;
  bool _preparedFast = false;
  bool _prunedLeafes = false;
};

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_DIRECTEDGRAPH_H
