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
    _adjacency[src].push_back(dst);
    if (_adjacency.count(dst) == 0) {
      _adjacency[dst].size();
    }
    _numEdges++;
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findAbove(
    uint64_t src) const {
  std::vector<uint64_t> tmp;

  const auto& it = _adjacency.find(src);

  if (it == _adjacency.end()) {
    return tmp;
  }

  std::vector<uint64_t> tmp2;
  for (const auto& dst : it->second) {
    tmp2.push_back(dst);
  }
  for (const auto& dst : it->second) {
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
std::vector<uint64_t> osm2ttl::util::DirectedGraph::findAboveFast(
    uint64_t src) const {
  const auto& it = _above.find(src);
  if (it == _adjacency.end()) {
    return std::vector<uint64_t>();
  }
  return it->second;
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
  const auto& vertices = getVertices();
#pragma omp parallel for shared(vertices) default(none)
  for (size_t i = 0; i < vertices.size(); i++) {
    std::sort(_adjacency[vertices[i]].begin(), _adjacency[vertices[i]].end());
  }
}
// ____________________________________________________________________________
void osm2ttl::util::DirectedGraph::prepareFastAbove() {
  const auto& vertices = getVertices();
  for (size_t i = 0; i < vertices.size(); i++) {
    _above[vertices[i]] = findAbove(i);
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