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

#ifndef OSM2RDF_OSM_DENSEMEMINDEX_H
#define OSM2RDF_OSM_DENSEMEMINDEX_H

#include <osmium/index/index.hpp>
#include <osmium/index/map.hpp>

namespace osm2rdf::osm {

template <typename TId, typename TValue>
class DenseMemIndex : public osmium::index::map::Map<TId, TValue> {
 public:
  explicit DenseMemIndex(size_t minNodeId, size_t maxNodeId);

  size_t size() const noexcept final { return _index.size(); }

  size_t used_memory() const noexcept final {
    return sizeof(DenseMemIndex) + _index.size() * sizeof(TValue);
  }

  void set(const TId id, const TValue value) final;

  TValue get_noexcept(const TId id) const noexcept final;

  TValue get(const TId id) const final;

  void clear() final;

  void sort() final{};

 private:
  size_t _offset;
  std::vector<TValue> _index;
};
}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_DENSEMEMINDEX_H
