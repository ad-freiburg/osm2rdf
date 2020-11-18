// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_DIRECTEDGRAPH_H
#define OSM2TTL_UTIL_DIRECTEDGRAPH_H

#include <stdint.h>

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace osm2ttl {
namespace util {

class DirectedGraph {
 public:
  // Type representing all vertices inside the directed graph.
  typedef uint64_t vertexID_t;
  // addEdge adds an edge between src and dst vertices. Allows multiple edges
  // between the same vertices.
  void addEdge(vertexID_t src, vertexID_t dst);
  // findSuccessors returns the ids of all successor vertices of the given
  // vertex.
  std::vector<vertexID_t> findSuccessors(vertexID_t src) const;
  // findSuccessorsFast returns the same result as findSuccessors but faster,
  // after data is prepared for faster lookup.
  std::vector<vertexID_t> findSuccessorsFast(vertexID_t src) const;
  // dump stores the complete graph in DOT-Format in a file at the given path.
  void dump(const std::filesystem::path& filename) const;
  // dumpOsm stores the complete graph in DOT-Format in a file at the given path.
  // This variant handles osm ids and converts for later lookup.
  void dumpOsm(const std::filesystem::path& filename) const;
  // prepareFindSuccessorsFast calculates for each vertex the successors and
  // stores them in a helper map.
  void prepareFindSuccessorsFast();
  // getNumEdges returns the number of stored edges. If an edge is defined
  // multiple times it is counted multiple times too.
  size_t getNumEdges() const;
  // getNumVertices returns the number of unique vertices in the graph.
  size_t getNumVertices() const;
  // getVertices returns all unique vertices in the graph.
  std::vector<osm2ttl::util::DirectedGraph::vertexID_t> getVertices() const;
  // getEdges returns the stored edges for the given vertex.
  std::vector<osm2ttl::util::DirectedGraph::vertexID_t> getEdges(
      vertexID_t src) const;

 protected:
  std::vector<vertexID_t> findSuccessorsHelper(vertexID_t src) const;
  std::unordered_map<vertexID_t, std::vector<vertexID_t>> _adjacency;
  std::unordered_map<vertexID_t, std::vector<vertexID_t>> _successors;
  size_t _numEdges = 0;
  bool preparedFast = false;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_DIRECTEDGRAPH_H
