// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_INCLUDE_OSM2TTL_UTIL_DIRECTEDGRAPH_H
#define OSM2TTL_INCLUDE_OSM2TTL_UTIL_DIRECTEDGRAPH_H

#include <stdint.h>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

namespace osm2ttl {
namespace util {

class DirectedGraph {
 public:
  void addEdge(uint64_t src, uint64_t dst);
  std::vector<uint64_t> findAbove(uint64_t src);
  std::vector<uint64_t> findBelow(uint64_t src);

 protected:
  std::vector<uint64_t> findInDirection(uint64_t src, bool up);
  std::unordered_map<uint64_t, std::vector<std::pair<uint64_t, bool>>> _adjacency;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_INCLUDE_OSM2TTL_UTIL_DIRECTEDGRAPH_H
