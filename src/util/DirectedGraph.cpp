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

#include "osm2rdf/util/DirectedGraph.h"

#include <stdint.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

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
void osm2rdf::util::DirectedGraph<T>::findSuccessorsHelper(
    T src, std::vector<T>* tmp) const {
  const auto& entry = _adjacency.find(src);
  if (entry == _adjacency.end()) {
    return;
  }

  // Copy direct successors.
  tmp->insert(tmp->end(), entry->second.begin(), entry->second.end());
  // Recursively add all successors.
  for (const auto& dst : entry->second) {
    findSuccessorsHelper(dst, tmp);
  }
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
  if (_prunedLeafes && entry->second.empty()) {
    // No result, if we are a leaf compute the result.
    const auto& adjacency = _adjacency.find(src);
    if (adjacency == _adjacency.end()) {
      return entry->second;
    }
    std::vector<T> successors;
    // Collect parents
    successors.insert(successors.end(), adjacency->second.begin(),
                      adjacency->second.end());
    for (const auto& successor : adjacency->second) {
      const auto& res = findSuccessorsFast(successor);
      successors.insert(successors.end(), res.begin(), res.end());
    }
    // Make unique
    std::sort(successors.begin(), successors.end());
    const auto it = std::unique(successors.begin(), successors.end());
    successors.resize(std::distance(successors.begin(), it));
    return successors;
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
  for (const auto& [vertex, _] : _adjacency) {
    const auto& successors = findSuccessors(vertex);
    if (successors.size()) _successors[vertex] = successors;
  }
  _preparedFast = true;
}

// ____________________________________________________________________________
template <typename T>
void osm2rdf::util::DirectedGraph<T>::prepareFindSuccessorsFastNoLeafes() {
  // for (const auto& vertex : getVertices()) {
  for (const auto& [vertex, _] : _adjacency) {
    auto i = _successors.find(vertex);
    if (i == _successors.end() || i->second.empty()) {
      for (const auto& successor : findSuccessors(vertex)) {
        auto j = _successors.find(successor);
        if (j == _successors.end() || j->second.empty()) {
          auto successors = findSuccessors(successor);;
          successors.shrink_to_fit();
          if (successors.size()) _successors[successor] = successors;
        }
      }
    }
  }
  _preparedFast = true;
  _prunedLeafes = true;
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
template <typename T>
std::vector<T> osm2rdf::util::DirectedGraph<T>::getEdgesFast(T src) const {
  if (!_preparedFast) {
    throw std::runtime_error("getEdgesFast not prepared");
  }
  const auto& res = _successors.find(src);
  if (_prunedLeafes && (res == _successors.end() || res->second.empty())) {
    std::vector<T> successors(_adjacency.at(src));
    for (const auto& s : successors) {
      const auto& res2 = _successors.find(s);
      if (res2 == _successors.end()) {
        continue;
      }
      successors.insert(successors.end(), res2->second.begin(),
                        res2->second.end());
    }
    // Make unique
    std::sort(successors.begin(), successors.end());
    const auto it = std::unique(successors.begin(), successors.end());
    successors.resize(std::distance(successors.begin(), it));
    return successors;
  }
  if (res == _successors.end()) return {};
  return res->second;
}

// ____________________________________________________________________________
template class osm2rdf::util::DirectedGraph<uint8_t>;
template class osm2rdf::util::DirectedGraph<uint16_t>;
template class osm2rdf::util::DirectedGraph<uint32_t>;
template class osm2rdf::util::DirectedGraph<uint64_t>;
