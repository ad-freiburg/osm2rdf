// Copyright 2024, University of Freiburg
// Authors: Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#include "osm2rdf/osm/DenseMemIndex.h"
#include "osm2rdf/osm/Location.h"
#include "osmium/osm/node.hpp"
#include <iostream>

// ____________________________________________________________________________
template <typename TId, typename TValue>
osm2rdf::osm::DenseMemIndex<TId, TValue>::DenseMemIndex(size_t minNodeId,
                                                        size_t maxNodeId)
    : _offset(minNodeId), _index(maxNodeId - minNodeId + 1) {
      std::cerr << minNodeId << " " << maxNodeId << std::endl;
}

// ____________________________________________________________________________
template <typename TId, typename TValue>
void osm2rdf::osm::DenseMemIndex<TId, TValue>::set(const TId id,
                                                   const TValue value) {
  assert(id >= _offset);
  assert(id < _index.size() + _offset);
  _index[id - _offset] = value;
}

// ____________________________________________________________________________
template <typename TId, typename TValue>
TValue osm2rdf::osm::DenseMemIndex<TId, TValue>::get_noexcept(
    const TId id) const noexcept {
  return _index[id - _offset];
}

// ____________________________________________________________________________
template <typename TId, typename TValue>
TValue osm2rdf::osm::DenseMemIndex<TId, TValue>::get(const TId id) const {
  if (id < _offset) {
    throw osmium::not_found{id};
  }
  if (id >= _index.size() + _offset) {
    throw osmium::not_found{id};
  }
  const auto value = get_noexcept(id);
  if (value == osmium::index::empty_value<TValue>()) {
    throw osmium::not_found{id};
  }
  return value;
}

// ____________________________________________________________________________
template <typename TId, typename TValue>
void osm2rdf::osm::DenseMemIndex<TId, TValue>::clear() {
  _index.clear();
  _index.shrink_to_fit();
  _offset = 0;
}

template class osm2rdf::osm::DenseMemIndex<osmium::unsigned_object_id_type,
                                           osm2rdf::osm::Location>;
