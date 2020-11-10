// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/DirectedGraph.h"

#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// ____________________________________________________________________________
void osm2ttl::util::DirectedGraph::addEdge(
    osm2ttl::util::DirectedGraph::VertexID src,
    osm2ttl::util::DirectedGraph::VertexID dst) {
  _adjacency[src].push_back(dst);
  if (_adjacency.count(dst) == 0) {
    _adjacency[dst].size();
  }
  _numEdges++;
}

// ____________________________________________________________________________
std::vector<osm2ttl::util::DirectedGraph::VertexID>
osm2ttl::util::DirectedGraph::findSuccessors(
    osm2ttl::util::DirectedGraph::VertexID src) const {
  std::vector<osm2ttl::util::DirectedGraph::VertexID> tmp;
  const auto& it = _adjacency.find(src);
  if (it == _adjacency.end()) {
    return tmp;
  }

  // Copy direct parents.
  tmp.insert(tmp.end(), it->second.begin(), it->second.end());
  // Add parents parents.
  for (const auto& dst : it->second) {
    const auto& v = findSuccessorsHelper(dst);
    tmp.insert(tmp.end(), v.begin(), v.end());
  }
  // Make unique
  std::sort(tmp.begin(), tmp.end());
  const auto it2 = std::unique(tmp.begin(), tmp.end());
  tmp.resize(std::distance(tmp.begin(), it2));
  return tmp;
}

// ____________________________________________________________________________
std::vector<osm2ttl::util::DirectedGraph::VertexID>
osm2ttl::util::DirectedGraph::findSuccessorsHelper(
    osm2ttl::util::DirectedGraph::VertexID src) const {
  std::vector<osm2ttl::util::DirectedGraph::VertexID> tmp;
  const auto& it = _adjacency.find(src);
  if (it == _adjacency.end()) {
    return tmp;
  }

  // Copy direct parents.
  tmp.insert(tmp.end(), it->second.begin(), it->second.end());
  // Add parents parents.
  for (const auto& dst : it->second) {
    const auto& v = findSuccessors(dst);
    tmp.insert(tmp.end(), v.begin(), v.end());
  }
  return tmp;
}

// ____________________________________________________________________________
std::vector<osm2ttl::util::DirectedGraph::VertexID>
osm2ttl::util::DirectedGraph::findSuccessorsFast(
    osm2ttl::util::DirectedGraph::VertexID src) const {
  const auto& it = _successors.find(src);
  if (it == _successors.end()) {
    return std::vector<osm2ttl::util::DirectedGraph::VertexID>();
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
void osm2ttl::util::DirectedGraph::prepareFindSuccessorsFast() {
  const auto& vertices = getVertices();
  for (size_t i = 0; i < vertices.size(); i++) {
    _successors[vertices[i]] = findSuccessors(vertices[i]);
  }
}

// ____________________________________________________________________________
size_t osm2ttl::util::DirectedGraph::getNumEdges() const { return _numEdges; }

// ____________________________________________________________________________
size_t osm2ttl::util::DirectedGraph::getNumVertices() const {
  return _adjacency.size();
}

// ____________________________________________________________________________
std::vector<osm2ttl::util::DirectedGraph::VertexID>
osm2ttl::util::DirectedGraph::getVertices() const {
  std::vector<osm2ttl::util::DirectedGraph::VertexID> result;
  for (const auto& [key, _] : _adjacency) {
    result.push_back(key);
  }
  return result;
}

// ____________________________________________________________________________
std::vector<osm2ttl::util::DirectedGraph::VertexID>
osm2ttl::util::DirectedGraph::getEdges(
    osm2ttl::util::DirectedGraph::VertexID src) const {
  const auto& tmp = _adjacency.at(src);
  return std::move(std::vector<osm2ttl::util::DirectedGraph::VertexID>(
      tmp.begin(), tmp.end()));
}