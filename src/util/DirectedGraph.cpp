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
#pragma omp critical(addEdge)
  {
    _adjacency[src].push_back(dst);
    if (_adjacency.count(dst) == 0) {
      _adjacency[dst].size();
    }
    _numEdges++;
  }
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findAbove(
    uint64_t src) const {
  std::vector<uint64_t> tmp;

  if (_adjacency.count(src) == 0) {
    return tmp;
  }

  std::vector<uint64_t> tmp2;
  for (const auto& dst : _adjacency.at(src)) {
    tmp2.push_back(dst);
  }
  for (const auto& dst : _adjacency.at(src)) {
    auto v = findAbove(dst);
    tmp2.insert(tmp2.end(), v.begin(), v.end());
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
    ofs << src << " [label=\"" << src << "\", shape=" << shape
        << ", style=solid]\n";
    for (const auto& dstId : list) {
      uint64_t dst = dstId;
      if ((dst & 1) == 1) {
        dst -= 1;
      }
      dst /= 2;
      ofs << src << " -> " << dst << "\n";
    }
  }
  ofs << "}\n";
  ofs.flush();
  ofs.close();
}
// ____________________________________________________________________________
void osm2ttl::util::DirectedGraph::sort() {
  std::vector<uint64_t> vertices = getVertices();
#pragma omp parallel for
  for (size_t i = 0; i < vertices.size(); i++) {
    std::sort(_adjacency[vertices[i]].begin(), _adjacency[vertices[i]].end());
  }
}

// ____________________________________________________________________________
size_t osm2ttl::util::DirectedGraph::getNumEdges() const {
  return _numEdges;
}

// ____________________________________________________________________________
size_t osm2ttl::util::DirectedGraph::getNumVertices() const {
  return _adjacency.size();
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::getVertices() const {
  std::vector<uint64_t> result;
  for (const auto& [key, _] : _adjacency) {
    result.push_back(key);
  }
  return result;
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::getEdges(uint64_t src) const {
  const auto& tmp = _adjacency.at(src);
  return std::move(std::vector<uint64_t>(tmp.begin(), tmp.end()));
}