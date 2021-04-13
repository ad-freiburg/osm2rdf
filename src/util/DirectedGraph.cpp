// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/util/DirectedGraph.h"

#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// ____________________________________________________________________________
template <typename T>
void osm2ttl::util::DirectedGraph<T>::addEdge(T src, T dst) {
  _adjacency[src].push_back(dst);
  if (_adjacency.count(dst) == 0) {
    _adjacency[dst].size();
  }
  _numEdges++;
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2ttl::util::DirectedGraph<T>::findSuccessors(T src) const {
  std::vector<T> tmp;
  // Collect parents
  findSuccessorsHelper(src, &tmp);
  // Make unique
  std::sort(tmp.begin(), tmp.end());
  const auto it2 = std::unique(tmp.begin(), tmp.end());
  tmp.resize(std::distance(tmp.begin(), it2));
  return tmp;
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::util::DirectedGraph<T>::findSuccessorsHelper(
    T src, std::vector<T>* tmp) const {
  const auto& it = _adjacency.find(src);
  if (it == _adjacency.end()) {
    return;
  }

  // Copy direct parents.
  tmp->insert(tmp->end(), it->second.begin(), it->second.end());
  // Add parents parents.
  for (const auto& dst : it->second) {
    findSuccessorsHelper(dst, tmp);
  }
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2ttl::util::DirectedGraph<T>::findSuccessorsFast(
    T src) const {
  if (!_preparedFast) {
    throw std::runtime_error("findSuccessorsFast not prepared");
  }
  const auto& it = _successors.find(src);
  if (it == _successors.end()) {
    return std::vector<T>();
  }
  return it->second;
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::util::DirectedGraph<T>::dump(
    const std::filesystem::path& filename) const {
  std::ofstream ofs(filename, std::ofstream::out);

  ofs << "digraph osm2ttl {\nrankdir=\"BT\"\n";
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
void osm2ttl::util::DirectedGraph<T>::dumpOsm(
    const std::filesystem::path& filename) const {
  std::ofstream ofs(filename, std::ofstream::out);

  ofs << "digraph osm2ttl {\nrankdir=\"BT\"\n";
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
void osm2ttl::util::DirectedGraph<T>::prepareFindSuccessorsFast() {
  const auto& vertices = getVertices();
  for (size_t i = 0; i < vertices.size(); i++) {
    _successors[vertices[i]] = findSuccessors(vertices[i]);
  }
  _preparedFast = true;
}

// ____________________________________________________________________________
template <typename T>
size_t osm2ttl::util::DirectedGraph<T>::getNumEdges() const {
  return _numEdges;
}

// ____________________________________________________________________________
template <typename T>
size_t osm2ttl::util::DirectedGraph<T>::getNumVertices() const {
  return _adjacency.size();
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2ttl::util::DirectedGraph<T>::getVertices() const {
  std::vector<T> result;
  for (const auto& [key, _] : _adjacency) {
    result.push_back(key);
  }
  return result;
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2ttl::util::DirectedGraph<T>::getEdges(T src) const {
  return _adjacency.at(src);
}

// ____________________________________________________________________________
template <typename T>
std::vector<T> osm2ttl::util::DirectedGraph<T>::getEdgesFast(T src) const {
  if (!_preparedFast) {
    throw std::runtime_error("findSuccessorsFast not prepared");
  }
  return _successors.at(src);
}

// ____________________________________________________________________________
template class osm2ttl::util::DirectedGraph<uint8_t>;
template class osm2ttl::util::DirectedGraph<uint16_t>;
template class osm2ttl::util::DirectedGraph<uint32_t>;
template class osm2ttl::util::DirectedGraph<uint64_t>;