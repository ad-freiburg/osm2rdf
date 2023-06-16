// Copyright 2023, University of Freiburg
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

#ifndef OSM2RDF_OSM_GEOMETRYCACHE_H_
#define OSM2RDF_OSM_GEOMETRYCACHE_H_

#include <fstream>
#include <iostream>
#include <list>
#include <unordered_map>

#include "omp.h"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Box.h"
#include "osm2rdf/geometry/Way.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Way.h"

namespace osm2rdf::osm {

typedef std::vector<osm2rdf::osm::Node::id_t> WayNodeList;

typedef std::pair<int32_t, uint8_t> BoxId;

struct BoxIdCmp {
  bool operator()(const BoxId& left, const BoxId& right) {
    return abs(left.first) < abs(right.first);
  }
  bool operator()(const BoxId& left, int32_t right) {
    return abs(left.first) < abs(right);
  }
};

typedef std::vector<BoxId> BoxIdList;

typedef std::tuple<
    osm2rdf::geometry::Box, osm2rdf::osm::Way::id_t, osm2rdf::geometry::Way,
    WayNodeList, std::vector<osm2rdf::geometry::Box>, osm2rdf::osm::BoxIdList,
    osm2rdf::geometry::Polygon, osm2rdf::geometry::Polygon, size_t, size_t>
    SpatialWayValue;

enum class AreaFromType { RELATION, WAY };

typedef std::tuple<std::vector<osm2rdf::geometry::Box>,                   // 0
                   osm2rdf::osm::Area::id_t,                              // 1
                   osm2rdf::geometry::Area,                               // 2
                   osm2rdf::osm::Area::id_t,                              // 3
                   osm2rdf::geometry::area_result_t,                      // 4
                   AreaFromType,                                          // 5
                   osm2rdf::geometry::Area,                               // 6
                   osm2rdf::geometry::Area,                               // 7
                   osm2rdf::osm::BoxIdList,                               // 8
                   std::unordered_map<int32_t, osm2rdf::geometry::Area>,  // 9
                   osm2rdf::geometry::Polygon,                            // 10
                   osm2rdf::geometry::Polygon>                            // 11
    SpatialAreaValue;

typedef std::tuple<osm2rdf::geometry::Area, osm2rdf::geometry::Area,
                   osm2rdf::geometry::Area, osm2rdf::geometry::Polygon>
    SpatialAreaValueCache;

template <typename W>
class GeometryCache {
 public:
  GeometryCache(size_t maxSize) : _maxSize(maxSize) {
    _geomsFReads.resize(omp_get_max_threads());
    _accessCount.resize(omp_get_max_threads(), 0);
    _diskAccessCount.resize(omp_get_max_threads(), 0);

    _vals.resize(omp_get_max_threads());
    _idMap.resize(omp_get_max_threads());

    _geomsF.open(getFName(), std::ios::out | std::ios::in | std::ios::binary |
                                 std::ios::trunc);
  }

  // ~GeometryCache() {
    // size_t access = 0;
    // size_t diskAccess = 0;
    // for (size_t i = 0; i < _accessCount.size(); i++) {
      // access += _accessCount[i];
      // diskAccess += _diskAccessCount[i];
    // }
    // std::cerr << "Geometry cache: " << access << " accesses, " << diskAccess
              // << " disk lookups" << std::endl;
  // }

  size_t add(const W& val);
  std::shared_ptr<W> get(size_t off) const;
  W getFromDisk(size_t off) const;
  std::shared_ptr<W> cache(size_t off, const W& val) const;

  void flush();

 private:
  std::string getFName() const;
  void readMultiPoly(std::fstream& str, osm2rdf::geometry::Area& ret) const;
  void writeMultiPoly(const osm2rdf::geometry::Area& ret);

  mutable std::vector<size_t> _accessCount;
  mutable std::vector<size_t> _diskAccessCount;

  mutable std::fstream _geomsF;
  mutable std::vector<std::fstream> _geomsFReads;
  size_t _geomsOffset = 0;

  mutable std::vector<std::list<std::pair<size_t, std::shared_ptr<W>>>> _vals;
  mutable std::vector<std::unordered_map<
      size_t,
      typename std::list<std::pair<size_t, std::shared_ptr<W>>>::iterator>>
      _idMap;

  size_t _maxSize;
};
}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_GEOMETRYCACHE_H_
