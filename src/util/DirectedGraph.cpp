// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DirectedGraph.h"

#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// ____________________________________________________________________________
void osm2ttl::util::DirectedGraph::addEdge(uint64_t src, uint64_t dst) {
  _adjacency[src].emplace_back(dst, true);
  _adjacency[dst].emplace_back(src, false);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findAbove(
    uint64_t src) const {
  return findInDirection(src, true);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findBelow(
    uint64_t src) const {
  return findInDirection(src, false);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findInDirection(
    uint64_t src, bool up) const {
  std::vector<uint64_t> tmp;

  if (_adjacency.count(src) == 0) {
    return tmp;
  }

  std::vector<uint64_t> tmp2;
  for (const auto& pair : _adjacency.at(src)) {
    if (pair.second == up) {
      tmp2.push_back(pair.first);
    }
  }
  for (const auto& pair : _adjacency.at(src)) {
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

// ____________________________________________________________________________
void osm2ttl::util::DirectedGraph::dump(std::filesystem::path filename) const {
  std::ofstream ofs;
  ofs.open(filename);

  ofs << "digraph osm2ttl {\nrankdir=\"BT\"\n";
  for (const auto& [id, list] : _adjacency) {
    uint64_t src = id;
    std::string shape = "rectangle";
    if ((src & 1) == 1) {
      shape = "ellipse";
      src -= 1;
    }
    src /= 2;
    ofs << src << " [label=\"" << src << "\", shape=" << shape << ", style=solid]\n";
    for (const auto& [dstId, dir] : list) {
      uint64_t dst = dstId;
      if (dir) {
        if ((dst & 1) == 1) {
          dst -= 1;
        }
        dst /= 2;
        ofs << src << " -> " << dst << "\n";
      }
    }
  }
  ofs << "}\n";
  ofs.flush();
  ofs.close();
}