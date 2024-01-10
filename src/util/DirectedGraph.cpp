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

#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <queue>

#include "osm2rdf/util/DirectedGraph.h"

// ____________________________________________________________________________
template <typename T>
void osm2rdf::util::DirectedGraph<T>::addEdge(T src, T dst) {
  _adjacency[src].push_back(dst);
  if (_adjacency.find(dst) == _adjacency.end()) {
    _adjacency[dst] = {};
  }
  _numEdges++;
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2rdf::util::DirectedGraph<T>::findSuccessors(T src) const {
  std::queue<T> q;
  std::set<T> visited;
  q.push(src);

  while (!q.empty()) {
    const auto& cur = q.front();
    visited.insert(cur);
    q.pop();

    const auto& entry = _adjacency.find(cur);
    if (entry == _adjacency.end()) {
      continue;
    }

    for (const auto& nd : entry->second) {
      if (visited.find(nd) == visited.end()) q.push(nd);
    }
  }

  visited.erase(src);

  std::vector<T> tmp;
  tmp.reserve(visited.size());
  tmp.insert(tmp.end(), visited.begin(), visited.end());

  // guaranteed to be sorted
  return tmp;
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2rdf::util::DirectedGraph<T>::findSuccessorsFast(
    T src) const {
  if (!_preparedFast) {
    throw std::runtime_error("findSuccessorsFast not prepared");
  }
  const auto& entry = _successors.find(src);
  if (entry == _successors.end()) {
    return std::vector<T>();
  }
  return entry->second;
}

// ____________________________________________________________________________
template <typename T>
void osm2rdf::util::DirectedGraph<T>::dump(
    const std::filesystem::path& filename) const {
  std::ofstream ofs(filename, std::ofstream::out);

  ofs << "digraph osm2rdf {\nrankdir=\"BT\"\n";
  for (const auto& [src, list] : _adjacency) {
    ofs << std::to_string(src) << " [label=\"" << std::to_string(src)
        << "\", shape=rectangle, style=solid]\n";
    for (const auto& dst : list) {
      ofs << std::to_string(src) << " -> " << std::to_string(dst) << "\n";
    }
  }
  ofs << "}\n";
  ofs.flush();
  ofs.close();
}

// ____________________________________________________________________________
template <typename T>
void osm2rdf::util::DirectedGraph<T>::dumpOsm(
    const std::filesystem::path& filename) const {
  std::ofstream ofs(filename, std::ofstream::out);

  ofs << "digraph osm2rdf {\nrankdir=\"BT\"\n";
  for (const auto& [id, list] : _adjacency) {
    uint64_t src = id;
    std::string shape = "rectangle";
    if ((src & 1U) == 1) {
      shape = "ellipse";
      src -= 1;
    }
    src /= 2;
    ofs << std::to_string(src) << " [label=\"" << std::to_string(src)
        << "\", shape=" << shape << ", style=solid]\n";
    for (const auto& dstId : list) {
      uint64_t dst = dstId;
      if ((dst & 1U) == 1) {
        dst -= 1;
      }
      dst /= 2;
      ofs << std::to_string(src) << " -> " << std::to_string(dst) << "\n";
    }
  }
  ofs << "}\n";
  ofs.flush();
  ofs.close();
}

// ____________________________________________________________________________
template <typename T>
void osm2rdf::util::DirectedGraph<T>::prepareFindSuccessorsFast() {
  const auto& vertices = getVertices();
  for (const auto& [key, _] : _adjacency) {
    _successors[key] = findSuccessors(key);
  }
  _preparedFast = true;
}

// ____________________________________________________________________________
template <typename T>
size_t osm2rdf::util::DirectedGraph<T>::getNumEdges() const {
  return _numEdges;
}

// ____________________________________________________________________________
template <typename T>
size_t osm2rdf::util::DirectedGraph<T>::getNumVertices() const {
  return _adjacency.size();
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2rdf::util::DirectedGraph<T>::getVertices() const {
  std::vector<T> result;
  for (const auto& [key, _] : _adjacency) {
    result.push_back(key);
  }
  return result;
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2rdf::util::DirectedGraph<T>::getEdges(T src) const {
  return _adjacency.at(src);
}

// ____________________________________________________________________________
template class osm2rdf::util::DirectedGraph<uint8_t>;
template class osm2rdf::util::DirectedGraph<uint16_t>;
template class osm2rdf::util::DirectedGraph<uint32_t>;
template class osm2rdf::util::DirectedGraph<uint64_t>;
