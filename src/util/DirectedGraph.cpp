// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DirectedGraph.h"

#include <stdint.h>
#include <algorithm>

// ____________________________________________________________________________
void osm2ttl::util::DirectedGraph::addEdge(uint64_t src, uint64_t dst) {
  _adjacency[src].emplace_back(dst, true);
  _adjacency[dst].emplace_back(src, false);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findAbove(uint64_t src) {
  return findInDirection(src, true);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findBelow(uint64_t src) {
  return findInDirection(src, false);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findInDirection(uint64_t src, bool up) {
  std::vector<uint64_t> tmp;

  if (_adjacency.count(src) == 0) {
    return tmp;
  }

  std::vector<uint64_t> tmp2;
  for (const auto& pair : _adjacency[src]) {
    if (pair.second == up) {
      tmp2.push_back(pair.first);
    }
  }
  for (const auto& pair : _adjacency[src]) {
    if (pair.second == up) {
      auto v = findInDirection(pair.first, up);
      tmp2.insert(tmp2.end(), v.begin(), v.end());
    }
  }

  for (uint64_t v : tmp2) {
    if (std::find(tmp.begin(), tmp.end(), v) == tmp.end()) {
      tmp.push_back(v);
    }
  }

  return std::move(std::vector<uint64_t>(tmp.rbegin(), tmp.rend()));
}