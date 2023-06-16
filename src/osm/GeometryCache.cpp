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
std::shared_ptr<W> osm2rdf::osm::GeometryCache<W>::get(size_t off) const {
  size_t tid = omp_get_thread_num();
  _accessCount[tid]++;

  // check if value is in cache
  auto it = _idMap[tid].find(off);
  if (it == _idMap[tid].end()) {
    // if not, load, cache and return
    const auto& ret = getFromDisk(off);
    return cache(off, ret);
  }

  // if in cache, move to front of list and return
  // splice only changes pointers in the linked list, no copying here
  _vals[tid].splice(_vals[tid].begin(), _vals[tid], it->second);
  return it->second->second;
}

// ____________________________________________________________________________
template <typename W>
std::shared_ptr<W> osm2rdf::osm::GeometryCache<W>::cache(size_t off,
                                                         const W& val) const {
  size_t tid = omp_get_thread_num();

  // push value to front
  _vals[tid].push_front({off, std::make_shared<W>(val)});

  // set map to front iterator
  _idMap[tid][off] = _vals[tid].begin();

  // if cache is too large, pop last element
  if (_vals[tid].size() > _maxSize) {
    auto last = _vals[tid].rbegin();
    _idMap[tid].erase(last->first);
    _vals[tid].pop_back();
  }

  return _vals[tid].front().second;
}

// ____________________________________________________________________________
template <>
osm2rdf::osm::SpatialWayValue
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>::getFromDisk(
    size_t off) const {
  size_t tid = omp_get_thread_num();

  _diskAccessCount[tid]++;
  osm2rdf::osm::SpatialWayValue ret;

  _geomsFReads[tid].seekg(off);

  size_t len;
  _geomsFReads[tid].read(reinterpret_cast<char*>(&len), sizeof(size_t));
  std::get<2>(ret).resize(len);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<2>(ret)[0]),
                         sizeof(osm2rdf::geometry::Location) * len);

  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<0>(ret)),
                         sizeof(osm2rdf::geometry::Box));

  // boxes
  _geomsFReads[tid].read(reinterpret_cast<char*>(&len), sizeof(size_t));
  std::get<4>(ret).resize(len);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<4>(ret)[0]),
                         sizeof(osm2rdf::geometry::Box) * len);

  // boxIds
  _geomsFReads[tid].read(reinterpret_cast<char*>(&len), sizeof(size_t));
  std::get<5>(ret).resize(len);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<5>(ret)[0]),
                         sizeof(osm2rdf::osm::BoxId) * len);

  // OBB
  std::get<7>(ret).outer().resize(5);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<7>(ret).outer()[0]),
                         sizeof(osm2rdf::geometry::Location) * 5);

  return ret;
}

// ____________________________________________________________________________
template <>
size_t osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>::add(
    const osm2rdf::osm::SpatialWayValue& val) {
  size_t ret = _geomsOffset;

  size_t size = std::get<2>(val).size();

  // geom
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<2>(val)[0]),
                sizeof(osm2rdf::geometry::Location) * size);

  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * size;

  // envelope
  _geomsF.write(reinterpret_cast<const char*>(&std::get<0>(val)),
                sizeof(osm2rdf::geometry::Box));

  _geomsOffset += sizeof(osm2rdf::geometry::Box);

  // boxes
  size = std::get<4>(val).size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<4>(val)[0]),
                sizeof(osm2rdf::geometry::Box) * size);

  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::geometry::Box) * size;

  // boxids
  size = std::get<5>(val).size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<5>(val)[0]),
                sizeof(osm2rdf::osm::BoxId) * size);

  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::osm::BoxId) * size;

  // OBB
  _geomsF.write(reinterpret_cast<const char*>(&std::get<7>(val).outer()[0]),
                sizeof(osm2rdf::geometry::Location) * 5);

  _geomsOffset += sizeof(osm2rdf::geometry::Location) * 5;

  // cache to avoid loading later on
  cache(ret, val);

  return ret;
}

// ____________________________________________________________________________
template <>
osm2rdf::osm::SpatialAreaValueCache
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>::getFromDisk(
    size_t off) const {
  size_t tid = omp_get_thread_num();
  _diskAccessCount[tid]++;
  osm2rdf::osm::SpatialAreaValueCache ret;

  _geomsFReads[tid].seekg(off);

  // geom
  readMultiPoly(_geomsFReads[tid], std::get<0>(ret));

  // inner
  readMultiPoly(_geomsFReads[tid], std::get<1>(ret));

  // outer
  readMultiPoly(_geomsFReads[tid], std::get<2>(ret));

  // convexhull
  size_t hullSize;
  _geomsFReads[tid].read(reinterpret_cast<char*>(&hullSize), sizeof(size_t));
  std::get<3>(ret).outer().resize(hullSize);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<3>(ret).outer()[0]),
                         sizeof(osm2rdf::geometry::Location) * hullSize);

  return ret;
}

// ____________________________________________________________________________
template <>
osm2rdf::osm::SpatialAreaValue
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValue>::getFromDisk(
    size_t off) const {
  size_t tid = omp_get_thread_num();
  _diskAccessCount[tid]++;
  osm2rdf::osm::SpatialAreaValue ret;

  _geomsFReads[tid].seekg(off);

  // geom
  readMultiPoly(_geomsFReads[tid], std::get<2>(ret));

  // envelopes
  size_t numEnvelopes;
  _geomsFReads[tid].read(reinterpret_cast<char*>(&numEnvelopes),
                         sizeof(size_t));
  std::get<0>(ret).resize(numEnvelopes);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<0>(ret)[0]),
                         sizeof(osm2rdf::geometry::Box) * numEnvelopes);

  // id
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<1>(ret)),
                         sizeof(id_t));

  // object ID
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<3>(ret)),
                         sizeof(id_t));

  // area
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<4>(ret)),
                         sizeof(osm2rdf::geometry::area_result_t));

  // fromWay?
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<5>(ret)),
                         sizeof(bool));

  // inner
  readMultiPoly(_geomsFReads[tid], std::get<6>(ret));

  // outer
  readMultiPoly(_geomsFReads[tid], std::get<7>(ret));

  // boxIds
  size_t numBoxIds;
  _geomsFReads[tid].read(reinterpret_cast<char*>(&numBoxIds), sizeof(size_t));
  std::get<8>(ret).resize(numBoxIds);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<8>(ret)[0]),
                         sizeof(osm2rdf::osm::BoxId) * numBoxIds);

  // cutouts
  size_t numCutouts;
  _geomsFReads[tid].read(reinterpret_cast<char*>(&numCutouts), sizeof(size_t));

  for (size_t i = 0; i < numCutouts; i++) {
    int32_t boxid;
    _geomsFReads[tid].read(reinterpret_cast<char*>(&boxid), sizeof(int32_t));
    readMultiPoly(_geomsFReads[tid], std::get<9>(ret)[boxid]);
  }

  // convexhull
  size_t hullSize;
  _geomsFReads[tid].read(reinterpret_cast<char*>(&hullSize), sizeof(size_t));
  std::get<10>(ret).outer().resize(hullSize);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<10>(ret).outer()[0]),
                         sizeof(osm2rdf::geometry::Location) * hullSize);

  // OBB
  std::get<11>(ret).outer().resize(5);
  _geomsFReads[tid].read(reinterpret_cast<char*>(&std::get<11>(ret).outer()[0]),
                         sizeof(osm2rdf::geometry::Location) * 5);

  return ret;
}

// ____________________________________________________________________________
template <>
size_t osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValue>::add(
    const osm2rdf::osm::SpatialAreaValue& val) {
  size_t ret = _geomsOffset;

  // geoms
  writeMultiPoly(std::get<2>(val));

  // envelopes
  size_t size = std::get<0>(val).size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<0>(val)[0]),
                sizeof(osm2rdf::geometry::Box) * size);
  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::geometry::Box) * size;

  // id
  _geomsF.write(reinterpret_cast<const char*>(&std::get<1>(val)), sizeof(id_t));
  _geomsOffset += sizeof(id_t);

  // object ID
  _geomsF.write(reinterpret_cast<const char*>(&std::get<3>(val)), sizeof(id_t));
  _geomsOffset += sizeof(id_t);

  // area
  _geomsF.write(reinterpret_cast<const char*>(&std::get<4>(val)),
                sizeof(osm2rdf::geometry::area_result_t));
  _geomsOffset += sizeof(osm2rdf::geometry::area_result_t);

  // fromWay?
  _geomsF.write(reinterpret_cast<const char*>(&std::get<5>(val)), sizeof(bool));
  _geomsOffset += sizeof(bool);

  // innerGeom
  writeMultiPoly(std::get<6>(val));

  // outerGeom
  writeMultiPoly(std::get<7>(val));

  // boxIds
  size = std::get<8>(val).size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<8>(val)[0]),
                sizeof(osm2rdf::osm::BoxId) * size);

  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::osm::BoxId) * size;

  // cutouts
  // std::unordered_map<int32_t, osm2rdf::geometry::Area
  size = std::get<9>(val).size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsOffset += sizeof(size_t);

  for (const auto& el : std::get<9>(val)) {
    int32_t boxid = el.first;
    const auto& cutout = el.second;
    size_t locSize = cutout.size();
    _geomsF.write(reinterpret_cast<const char*>(&boxid), sizeof(int32_t));
    _geomsOffset += sizeof(int32_t);
    writeMultiPoly(cutout);
  }

  // convexhull
  size = std::get<10>(val).outer().size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<10>(val).outer()[0]),
                sizeof(osm2rdf::geometry::Location) * size);
  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * size;

  // OBB
  _geomsF.write(reinterpret_cast<const char*>(&std::get<11>(val).outer()[0]),
                sizeof(osm2rdf::geometry::Location) * 5);
  _geomsOffset += sizeof(osm2rdf::geometry::Location) * 5;

  // cache to avoid loading later on
  cache(ret, val);

  return ret;
}

// ____________________________________________________________________________
template <>
size_t osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>::add(
    const osm2rdf::osm::SpatialAreaValueCache& val) {
  size_t ret = _geomsOffset;

  // geoms
  writeMultiPoly(std::get<0>(val));

  // innerGeom
  writeMultiPoly(std::get<1>(val));

  // outerGeom
  writeMultiPoly(std::get<2>(val));

  // convexhull
  size_t size = std::get<3>(val).outer().size();
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsF.write(reinterpret_cast<const char*>(&std::get<3>(val).outer()[0]),
                sizeof(osm2rdf::geometry::Location) * size);
  _geomsOffset += sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * size;

  // cache to avoid loading later on
  cache(ret, val);

  return ret;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryCache<W>::flush() {
  if (_geomsF.is_open()) {
    _geomsF.flush();
  }

  for (size_t i = 0; i < omp_get_max_threads(); i++) {
    _geomsFReads[i].open(getFName(), std::ios::in | std::ios::binary);
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
  _geomsF.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
  _geomsOffset += sizeof(size_t);

  for (const auto& geom : val) {
    // geom, outer
    size_t locSize = geom.outer().size();
    _geomsF.write(reinterpret_cast<const char*>(&locSize), sizeof(size_t));
    _geomsF.write(reinterpret_cast<const char*>(&geom.outer()[0]),
                  sizeof(osm2rdf::geometry::Location) * locSize);
    _geomsOffset +=
        sizeof(size_t) + sizeof(osm2rdf::geometry::Location) * locSize;

    // geom, inners
    locSize = geom.inners().size();
    _geomsF.write(reinterpret_cast<const char*>(&locSize), sizeof(size_t));
    _geomsOffset += sizeof(size_t);

    for (const auto& inner : geom.inners()) {
      locSize = inner.size();
      _geomsF.write(reinterpret_cast<const char*>(&locSize), sizeof(size_t));
      _geomsF.write(reinterpret_cast<const char*>(&inner[0]),
                    sizeof(osm2rdf::geometry::Location) * locSize);
      _geomsOffset +=
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
std::string osm2rdf::osm::GeometryCache<
    osm2rdf::osm::SpatialAreaValueCache>::getFName() const {
  return "areageomslight";
}

// ____________________________________________________________________________
template <>
std::string
osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValue>::getFName() const {
  return "areageoms";
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialWayValue>;
template class osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValueCache>;
template class osm2rdf::osm::GeometryCache<osm2rdf::osm::SpatialAreaValue>;
