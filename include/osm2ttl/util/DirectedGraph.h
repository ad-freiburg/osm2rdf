// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_INCLUDE_OSM2TTL_UTIL_DIRECTEDGRAPH_H
#define OSM2TTL_INCLUDE_OSM2TTL_UTIL_DIRECTEDGRAPH_H

#include <stdint.h>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace osm2ttl {
namespace util {

class DirectedGraph {
 public:
  void addEdge(uint64_t src, uint64_t dst);
  std::vector<uint64_t> findAbove(uint64_t src) const;
  std::vector<uint64_t> findAboveFast(uint64_t src) const;
  void dump(std::filesystem::path filename) const;
  void sort();
  void prepareFastAbove();
  size_t getNumEdges() const;
  size_t getNumVertices() const;
  std::vector<uint64_t> getVertices() const;
  std::vector<uint64_t> getEdges(uint64_t src) const;

 protected:
  std::vector<uint64_t> findAboveHelper(uint64_t src) const;
  std::unordered_map<uint64_t, std::vector<uint64_t>> _adjacency;
  std::unordered_map<uint64_t, std::vector<uint64_t>> _above;
  size_t _numEdges = 0;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_INCLUDE_OSM2TTL_UTIL_DIRECTEDGRAPH_H
