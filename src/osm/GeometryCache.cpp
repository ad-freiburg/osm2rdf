// Copyright 2023, University of Freiburg
// Authors: Patrick Brosi <brosi@cs.uni-freiburg.de>

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

#include <fstream>
#include <iostream>

#include "omp.h"
#include "osm2rdf/geometry/Box.h"
#include "osm2rdf/geometry/Way.h"
#include "osm2rdf/osm/GeometryCache.h"
#include "osm2rdf/osm/Way.h"

// ____________________________________________________________________________
template <typename W>
W osm2rdf::osm::GeometryCache<W>::get(size_t off) const {
  _accessCount[omp_get_thread_num()]++;

  // check if value is in cache
  auto it = _idMap[omp_get_thread_num()].find(off);
  if (it == _idMap[omp_get_thread_num()].end()) {
    // if not, load, cache and return
    const auto& ret = getFromDisk(off);
    cache(off, ret);
    return ret;
  }

  // if in cache, move to front of list and return
  // _vals[omp_get_thread_num()].splice(_vals[omp_get_thread_num()].begin(),
                                     // _vals[omp_get_thread_num()], it->second);
  return it->second->second;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryCache<W>::cache(size_t off, const W& val) const {
  // push value to front
  _vals[omp_get_thread_num()].push_front({off, val});

  // set map to front iterator
  _idMap[omp_get_thread_num()][off] = _vals[omp_get_thread_num()].begin();

  // if cache is too large, pop last element
  if (_vals[omp_get_thread_num()].size() > _maxSize) {
    auto last = _vals[omp_get_thread_num()].rbegin();
    _idMap[omp_get_thread_num()].erase(last->first);
    _vals[omp_get_thread_num()].pop_back();
  }
}

// ____________________________________________________________________________
template <>
osm2rdf::osm::SpatialWayValue
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>::getFromDisk(
    size_t off) const {
  _diskAccessCount[omp_get_thread_num()]++;
  osm2rdf::osm::SpatialWayValue ret;

  _waysGeomsFReads[omp_get_thread_num()].seekg(off);

  size_t len;
  _waysGeomsFReads[omp_get_thread_num()].read(reinterpret_cast<char*>(&len),
                                              sizeof(size_t));
  std::get<2>(ret).resize(len);
  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&std::get<2>(ret)[0]),
      sizeof(osm2rdf::geometry::Location) * len);

  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&std::get<0>(ret)),
      sizeof(osm2rdf::geometry::Box));

  // boxes
  _waysGeomsFReads[omp_get_thread_num()].read(reinterpret_cast<char*>(&len),
                                              sizeof(size_t));
  std::get<4>(ret).resize(len);
  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&std::get<4>(ret)[0]),
      sizeof(osm2rdf::geometry::Box) * len);

  // boxIds
  _waysGeomsFReads[omp_get_thread_num()].read(reinterpret_cast<char*>(&len),
                                              sizeof(size_t));
  std::get<5>(ret).resize(len);
  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&std::get<5>(ret)[0]),
      sizeof(osm2rdf::osm::BoxId) * len);

  // OBB
  std::get<7>(ret).outer().resize(5);
  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&std::get<7>(ret).outer()[0]),
      sizeof(osm2rdf::geometry::Location) * 5);

  return ret;
}

// ____________________________________________________________________________
template <>
size_t osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>::add(
    const osm2rdf::osm::SpatialWayValue& val) {
  size_t ret = _waysGeomsOffset;

  size_t size = std::get<2>(val).size();

  // geom
  _waysGeomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _waysGeomsF.write(reinterpret_cast<const char*>(&std::get<2>(val)[0]),
                    sizeof(osm2rdf::geometry::Location) * size);

  _waysGeomsOffset +=
      sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * size;

  // envelope
  _waysGeomsF.write(reinterpret_cast<const char*>(&std::get<0>(val)),
                    sizeof(osm2rdf::geometry::Box));

  _waysGeomsOffset += sizeof(osm2rdf::geometry::Box);

  // boxes
  size = std::get<4>(val).size();
  _waysGeomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _waysGeomsF.write(reinterpret_cast<const char*>(&std::get<4>(val)[0]),
                    sizeof(osm2rdf::geometry::Box) * size);

  _waysGeomsOffset += sizeof(size_t) + sizeof(osm2rdf::geometry::Box) * size;

  // boxids
  size = std::get<5>(val).size();
  _waysGeomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _waysGeomsF.write(reinterpret_cast<const char*>(&std::get<5>(val)[0]),
                    sizeof(osm2rdf::osm::BoxId) * size);

  _waysGeomsOffset += sizeof(size_t) + sizeof(osm2rdf::osm::BoxId) * size;

  // OBB
  _waysGeomsF.write(reinterpret_cast<const char*>(&std::get<7>(val).outer()[0]),
                    sizeof(osm2rdf::geometry::Location) * 5);

  _waysGeomsOffset += sizeof(osm2rdf::geometry::Location) * 5;

  return ret;
}

// ____________________________________________________________________________
template <>
osm2rdf::osm::SpatialAreaValueCache
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>::getFromDisk(
    size_t off) const {
  _diskAccessCount[omp_get_thread_num()]++;
  osm2rdf::osm::SpatialAreaValueCache ret;

  _waysGeomsFReads[omp_get_thread_num()].seekg(off);

  // geom
  readMultiPoly(_waysGeomsFReads[omp_get_thread_num()], std::get<0>(ret));

  // inner
  readMultiPoly(_waysGeomsFReads[omp_get_thread_num()], std::get<1>(ret));

  // outer
  readMultiPoly(_waysGeomsFReads[omp_get_thread_num()], std::get<2>(ret));

  // convexhull
  size_t hullSize;
  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&hullSize), sizeof(size_t));
  std::get<3>(ret).outer().resize(hullSize);
  _waysGeomsFReads[omp_get_thread_num()].read(
      reinterpret_cast<char*>(&std::get<3>(ret).outer()[0]),
      sizeof(osm2rdf::geometry::Location) * hullSize);

  return ret;
}

// ____________________________________________________________________________
template <>
size_t osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>::add(
    const osm2rdf::osm::SpatialAreaValueCache& val) {
  size_t ret = _waysGeomsOffset;

  // geoms
  writeMultiPoly(std::get<0>(val));

  // innerGeom
  writeMultiPoly(std::get<1>(val));

  // outerGeom
  writeMultiPoly(std::get<2>(val));


  // convexhull
  size_t size = std::get<3>(val).outer().size();
  _waysGeomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _waysGeomsF.write(
      reinterpret_cast<const char*>(&std::get<3>(val).outer()[0]),
      sizeof(osm2rdf::geometry::Location) * size);
  _waysGeomsOffset +=
      sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * size;

  return ret;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryCache<W>::flush() {
  if (_waysGeomsF.is_open()) {
    _waysGeomsF.flush();
  }

  for (size_t i = 0; i < omp_get_max_threads(); i++) {
    _waysGeomsFReads[i].open(getFName(), std::ios::in | std::ios::binary);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryCache<W>::readMultiPoly(
    std::fstream& str, osm2rdf::geometry::Area& ret) const {
  size_t numPolygons;
  str.read(reinterpret_cast<char*>(&numPolygons), sizeof(size_t));
  ret.resize(numPolygons);

  for (size_t i = 0; i < numPolygons; i++) {
    size_t sizeOuter;
    str.read(reinterpret_cast<char*>(&sizeOuter), sizeof(size_t));
    ret[i].outer().resize(sizeOuter);

    str.read(reinterpret_cast<char*>(&ret[i].outer()[0]),
             sizeof(osm2rdf::geometry::Location) * sizeOuter);

    size_t numInners;
    str.read(reinterpret_cast<char*>(&numInners), sizeof(size_t));
    ret[i].inners().resize(numInners);

    for (size_t j = 0; j < numInners; j++) {
      size_t sizeInner;
      str.read(reinterpret_cast<char*>(&sizeInner), sizeof(size_t));

      ret[i].inners()[j].resize(sizeInner);

      str.read(reinterpret_cast<char*>(&ret[i].inners()[j][0]),
               sizeof(osm2rdf::geometry::Location) * sizeInner);
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryCache<W>::writeMultiPoly(
    const osm2rdf::geometry::Area& val) {
  size_t size = val.size();
  _waysGeomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _waysGeomsOffset += sizeof(size_t);

  for (const auto& geom : val) {
    // geom, outer
    size_t locSize = geom.outer().size();
    _waysGeomsF.write(reinterpret_cast<const char*>(&locSize), sizeof(size_t));
    _waysGeomsF.write(reinterpret_cast<const char*>(&geom.outer()[0]),
                      sizeof(osm2rdf::geometry::Location) * locSize);
    _waysGeomsOffset +=
        sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * locSize;

    // geom, inners
    locSize = geom.inners().size();
    _waysGeomsF.write(reinterpret_cast<const char*>(&locSize), sizeof(size_t));
    _waysGeomsOffset += sizeof(size_t);

    for (const auto& inner : geom.inners()) {
      locSize = inner.size();
      _waysGeomsF.write(reinterpret_cast<const char*>(&locSize),
                        sizeof(size_t));
      _waysGeomsF.write(reinterpret_cast<const char*>(&inner[0]),
                        sizeof(osm2rdf::geometry::Location) * locSize);
      _waysGeomsOffset +=
          sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * locSize;
    }
  }
}

// ____________________________________________________________________________
template <>
std::string
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>::getFName() const {
  return "waygeoms";
}

// ____________________________________________________________________________
template <>
std::string
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>::getFName() const {
  return "areageoms";
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>;
template class osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>;
