// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#ifndef OSM2RDF_OSM_GEOMETRYHANDLER_H_
#define OSM2RDF_OSM_GEOMETRYHANDLER_H_

#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "boost/archive/binary_oarchive.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "gtest/gtest_prod.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/geometry/Node.h"
#include "osm2rdf/geometry/Way.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/CacheFile.h"
#include "osm2rdf/util/DirectedGraph.h"
#include "osm2rdf/util/Output.h"

namespace osm2rdf::osm {

const static int NUM_GRID_CELLS = 5000;

const static double GRID_W = 360.0 / NUM_GRID_CELLS;
const static double GRID_H = 180.0 / NUM_GRID_CELLS;

struct GeomRelationStats {
  size_t _totalChecks = 0;
  size_t _fullChecks = 0;
  size_t _skippedByNonIntersect = 0;
  size_t _skippedByDAG = 0;
  size_t _skippedByAreaSize = 0;
  size_t _skippedByBoxIdIntersect = 0;
  size_t _skippedByBoxIdIntersectCutout = 0;
  size_t _skippedByContainedInInnerRing = 0;
  size_t _skippedByBorderContained = 0;
  size_t _skippedByNodeContained = 0;
  size_t _skippedByInner = 0;
  size_t _skippedByOuter = 0;
  size_t _skippedByBox = 0;
  size_t _skippedByConvexHull = 0;

  GeomRelationStats& operator+=(const GeomRelationStats& lh) {
    _totalChecks += lh._totalChecks;
    _fullChecks += lh._fullChecks;
    _skippedByNonIntersect += lh._skippedByNonIntersect;
    _skippedByDAG += lh._skippedByDAG;
    _skippedByAreaSize += lh._skippedByAreaSize;
    _skippedByBoxIdIntersect += lh._skippedByBoxIdIntersect;
    _skippedByBoxIdIntersectCutout += lh._skippedByBoxIdIntersectCutout;
    _skippedByContainedInInnerRing += lh._skippedByContainedInInnerRing;
    _skippedByInner += lh._skippedByInner;
    _skippedByOuter += lh._skippedByOuter;
    _skippedByBox += lh._skippedByBox;
    _skippedByConvexHull += lh._skippedByConvexHull;
    _skippedByBorderContained += lh._skippedByBorderContained;
    _skippedByNodeContained += lh._skippedByNodeContained;
    return *this;
  }

  void checked() { _totalChecks++; }
  void skippedByNonIntersect() { _skippedByNonIntersect++; }
  void skippedByDAG() { _skippedByDAG++; }
  void skippedByAreaSize() { _skippedByAreaSize++; }
  void skippedByBoxIdIntersect() { _skippedByBoxIdIntersect++; }
  void skippedByBoxIdIntersectCutout() { _skippedByBoxIdIntersectCutout++; }
  void skippedByContainedInInnerRing() { _skippedByContainedInInnerRing++; }
  void skippedByInner() { _skippedByInner++; }
  void skippedByOuter() { _skippedByOuter++; }
  void skippedByBox() { _skippedByBox++; }
  void skippedByConvexHull() { _skippedByConvexHull++; }
  void skippedByBorderContained() { _skippedByBorderContained++; }
  void skippedByNodeContained() { _skippedByNodeContained++; }
  void fullCheck() { _fullChecks++; }

  std::string printPercNum(size_t n) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << n << " ("
       << ((static_cast<double>(n) / static_cast<double>(_totalChecks)) * 100.0)
       << "%)";
    return ss.str();
  }

  std::string printTotalChecks() const { return std::to_string(_totalChecks); }
  std::string printSkippedByDAG() const { return printPercNum(_skippedByDAG); }
  std::string printSkippedByAreaSize() const {
    return printPercNum(_skippedByAreaSize);
  }
  std::string printSkippedByBox() const { return printPercNum(_skippedByBox); }
  std::string printSkippedByBoxIdIntersect() const {
    return printPercNum(_skippedByBoxIdIntersect);
  }
  std::string printSkippedByNonIntersect() const {
    return printPercNum(_skippedByNonIntersect);
  }
  std::string printSkippedByBoxIdIntersectCutout() const {
    return printPercNum(_skippedByBoxIdIntersectCutout);
  }
  std::string printSkippedByContainedInInnerRing() const {
    return printPercNum(_skippedByContainedInInnerRing);
  }
  std::string printSkippedByConvexHull() const {
    return printPercNum(_skippedByConvexHull);
  }
  std::string printSkippedByBorderContained() const {
    return printPercNum(_skippedByBorderContained);
  }
  std::string printSkippedByInner() const {
    return printPercNum(_skippedByInner);
  }
  std::string printSkippedByOuter() const {
    return printPercNum(_skippedByOuter);
  }
  std::string printSkippedByNodeContained() const {
    return printPercNum(_skippedByNodeContained);
  }
  std::string printFullChecks() const { return printPercNum(_fullChecks); }
};

#pragma omp declare \
reduction(  \
        + : \
            GeomRelationStats :  \
                omp_out += omp_in ) \
initializer( omp_priv = omp_orig )

typedef std::pair<int32_t, uint8_t> BoxId;

enum RelInfoValue { DONTKNOW, YES, NO };

struct GeomRelationInfo {
  RelInfoValue intersects = DONTKNOW;
  RelInfoValue contained = DONTKNOW;

  double intersectArea = -1;

  std::vector<int32_t> toCheck;
  int fullContained = -1;
};

typedef std::unordered_set<
    osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
    SkipSet;

typedef std::pair<osm2rdf::osm::Area::id_t, bool> MemberRel;

struct MemberRelCmp {
  bool operator()(const MemberRel& left, const MemberRel& right) {
    return left.first < right.first;
  }
  bool operator()(const MemberRel& left, osm2rdf::osm::Area::id_t right) {
    return left.first < right;
  }
  bool operator()(osm2rdf::osm::Area::id_t left, const MemberRel& right) {
    return left < right.first;
  }
};

struct BoxIdCmp {
  bool operator()(const BoxId& left, const BoxId& right) {
    return abs(left.first) < abs(right.first);
  }
  bool operator()(const BoxId& left, int32_t right) {
    return abs(left.first) < abs(right);
  }
};

typedef std::vector<BoxId> BoxIdList;

// Area: envelope, id, geometry, osm id, area, fromWay
typedef std::tuple<std::vector<osm2rdf::geometry::Box>,
                   osm2rdf::osm::Area::id_t, osm2rdf::geometry::Area,
                   osm2rdf::osm::Area::id_t, osm2rdf::geometry::area_result_t,
                   uint8_t, osm2rdf::geometry::Area, osm2rdf::geometry::Area,
                   osm2rdf::osm::BoxIdList,
                   std::unordered_map<int32_t, osm2rdf::geometry::Area>,
                   osm2rdf::geometry::Polygon>
    SpatialAreaValue;

typedef std::pair<osm2rdf::geometry::Box, size_t> SpatialAreaRefValue;

typedef std::vector<SpatialAreaValue> SpatialAreaVector;

// Node: envelope, osm  id, geometry
typedef std::tuple<osm2rdf::osm::Node::id_t, osm2rdf::geometry::Node>
    SpatialNodeValue;
typedef std::vector<SpatialNodeValue> SpatialNodeVector;

typedef std::vector<osm2rdf::osm::Node::id_t> WayNodeList;

// Way: envelope, osm id, geometry, node list
typedef std::tuple<osm2rdf::geometry::Box, osm2rdf::osm::Way::id_t,
                   osm2rdf::geometry::Way, WayNodeList,
                   std::vector<osm2rdf::geometry::Box>, osm2rdf::osm::BoxIdList>
    SpatialWayValue;
typedef std::vector<SpatialWayValue> SpatialWayVector;

typedef boost::geometry::index::rtree<SpatialAreaRefValue,
                                      boost::geometry::index::quadratic<32>>
    SpatialIndex;

// node osm id -> area ids (not osm id)
typedef std::unordered_map<osm2rdf::osm::Node::id_t,
                           std::vector<osm2rdf::osm::Area::id_t>>
    NodesContainedInAreasData;

template <typename W>
class GeometryHandler {
 public:
  GeometryHandler(const osm2rdf::config::Config& config,
                  osm2rdf::ttl::Writer<W>* writer);
  ~GeometryHandler();

  // Add data
  void area(const osm2rdf::osm::Area& area);
  void node(const osm2rdf::osm::Node& node);
  void way(const osm2rdf::osm::Way& way);
  void relation(const osm2rdf::osm::Relation& rel);
  // close external storage files
  void closeExternalStorage();
  // Calculate data
  void calculateRelations();

 protected:
  // Stores named areas in r-tree, used for all other calculations.
  void prepareRTree();
  FRIEND_TEST(OSM_GeometryHandler, prepareRTreeEmpty);
  FRIEND_TEST(OSM_GeometryHandler, prepareRTreeSimple);

  // Generate DAG for areas using prepared r-tree.
  void prepareDAG();
  FRIEND_TEST(OSM_GeometryHandler, prepareDAGEmpty);
  FRIEND_TEST(OSM_GeometryHandler, prepareDAGSimple);

  // Calculate relations for each area, this dumps the generated DAG.
  void dumpNamedAreaRelations();
  FRIEND_TEST(OSM_GeometryHandler, dumpNamedAreaRelationsEmpty);
  FRIEND_TEST(OSM_GeometryHandler, dumpNamedAreaRelationsSimple);
  FRIEND_TEST(OSM_GeometryHandler, dumpNamedAreaRelationsSimpleOpenMP);

  // Calculate relations for each area, this dumps the generated DAG.
  void dumpUnnamedAreaRelations();
  FRIEND_TEST(OSM_GeometryHandler, noAreaGeometricRelations);
  FRIEND_TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsEmpty1);
  FRIEND_TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsEmpty2);
  FRIEND_TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsSimpleIntersects);
  FRIEND_TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsSimpleContainsOnly);

  // Calculate relations for each node.
  NodesContainedInAreasData dumpNodeRelations();
  FRIEND_TEST(OSM_GeometryHandler, noNodeGeometricRelations);
  FRIEND_TEST(OSM_GeometryHandler, dumpNodeRelationsEmpty1);
  FRIEND_TEST(OSM_GeometryHandler, dumpNodeRelationsEmpty2);
  FRIEND_TEST(OSM_GeometryHandler, dumpNodeRelationsSimpleIntersects);
  FRIEND_TEST(OSM_GeometryHandler, dumpNodeRelationsSimpleContains);

  // Calculate relations for each way.
  void dumpWayRelations(
      const osm2rdf::osm::NodesContainedInAreasData& nodeData);
  FRIEND_TEST(OSM_GeometryHandler, noWayGeometricRelations);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsEmpty1);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsEmpty2);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleIntersects);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleContains);
  FRIEND_TEST(OSM_GeometryHandler,
              dumpWayRelationsSimpleIntersectsWithNodeInfo);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleContainsWithNodeInfo);

  template <typename G>
  [[nodiscard]] G simplifyGeometry(const G& g) const;
  FRIEND_TEST(OSM_GeometryHandler, simplifyGeometryArea);
  FRIEND_TEST(OSM_GeometryHandler, simplifyGeometryWay);

  bool areaInArea(const SpatialAreaValue& a, const SpatialAreaValue&,
                  GeomRelationInfo* geomRelInf, GeomRelationStats* stats) const;
  bool areaInAreaApprox(const SpatialAreaValue& a, const SpatialAreaValue&,
                        GeomRelationInfo* geomRelInf,
                        GeomRelationStats* stats) const;
  bool nodeInArea(const SpatialNodeValue& a, const SpatialAreaValue&,
                  GeomRelationInfo* geomRelInf, GeomRelationStats* statsa) const;
  bool wayInArea(const SpatialWayValue& a, const SpatialAreaValue&,
                 GeomRelationInfo* geomRelInf, GeomRelationStats* stats) const;
  bool wayIntersectsArea(const SpatialWayValue& a, const SpatialAreaValue&,
                         GeomRelationInfo* geomRelInf,
                         GeomRelationStats* stats) const;

  bool areaIntersectsArea(const SpatialAreaValue& a, const SpatialAreaValue&,
                          GeomRelationInfo* geomRelInf,
                          GeomRelationStats* stats) const;

  static double signedDistanceFromPointToLine(
      const osm2rdf::geometry::Location& A,
      const osm2rdf::geometry::Location& B,
      const osm2rdf::geometry::Location& C);

  template <int MODE>
  bool ioDouglasPeucker(
      const boost::geometry::model::ring<osm2rdf::geometry::Location>& input,
      boost::geometry::model::ring<osm2rdf::geometry::Location>& output,
      size_t l, size_t r, double eps) const;

  static int polygonOrientation(
      const boost::geometry::model::ring<osm2rdf::geometry::Location>&
          polygon);

  osm2rdf::geometry::Area simplifiedArea(const osm2rdf::geometry::Area& area,
                                         bool inner) const;

  std::string areaNS(uint8_t type) const;

  void getBoxIds(
      const osm2rdf::geometry::Area& area,
      const osm2rdf::geometry::Area& inner,
      const osm2rdf::geometry::Area& outer,
      const std::vector<osm2rdf::geometry::Box>& envelopes, int xFrom, int xTo,
      int yFrom, int yTo, int xWidth, int yHeight, osm2rdf::osm::BoxIdList* ret,
      const osm2rdf::geometry::Area& curISect,
      std::unordered_map<int32_t, osm2rdf::geometry::Area>* cutouts) const;

  osm2rdf::osm::BoxIdList getBoxIds(
      const osm2rdf::geometry::Area&,
      const std::vector<osm2rdf::geometry::Box>& envelopes,
      const osm2rdf::geometry::Area& inner,
      const osm2rdf::geometry::Area& outer,
      std::unordered_map<int32_t, osm2rdf::geometry::Area>* cutouts) const;

  osm2rdf::osm::BoxIdList getBoxIds(
      const osm2rdf::geometry::Way&,
      const osm2rdf::geometry::Box& envelope) const;

  int32_t getBoxId(const osm2rdf::geometry::Location&) const;

  void boxIdIsect(const osm2rdf::osm::BoxIdList& idsA,
                  const osm2rdf::osm::BoxIdList& idsB,
                  GeomRelationInfo* geomRelInf) const;
  osm2rdf::osm::BoxIdList pack(const osm2rdf::osm::BoxIdList& ids) const;

  uint8_t borderContained(osm2rdf::osm::Way::id_t wayId,
                          osm2rdf::osm::Area::id_t areaId) const;

  std::vector<SpatialAreaRefValue> indexQryCover(
      const SpatialAreaValue& area) const;
  std::vector<SpatialAreaRefValue> indexQry(const SpatialNodeValue& node) const;
  std::vector<SpatialAreaRefValue> indexQryIntersect(
      const SpatialAreaValue& area) const;
  std::vector<SpatialAreaRefValue> indexQryIntersect(
      const SpatialWayValue& way) const;
  void unique(std::vector<SpatialAreaRefValue>& refs) const;

  // Global config
  osm2rdf::config::Config _config;
  osm2rdf::ttl::Writer<W>* _writer;
  // Store areas as r-tree
  SpatialIndex _spatialIndex;
  // Store dag
  osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t> _directedAreaGraph;
  // Spatial Data
  SpatialAreaVector _spatialStorageArea;
  std::unordered_map<osm2rdf::osm::Area::id_t, uint64_t>
      _spatialStorageAreaIndex;

  std::unordered_map<osm2rdf::osm::Way::id_t, std::vector<MemberRel>>
      _areaBorderWaysIndex;

  FRIEND_TEST(OSM_GeometryHandler, addNamedAreaFromRelation);
  FRIEND_TEST(OSM_GeometryHandler, addNamedAreaFromWay);
  FRIEND_TEST(OSM_GeometryHandler, addNamedAreaFromRelationWithRatios);
  FRIEND_TEST(OSM_GeometryHandler, addNamedAreaFromWayWithRatios);

  size_t _numUnnamedAreas = 0;
  FRIEND_TEST(OSM_GeometryHandler, addUnnamedAreaFromRelation);
  FRIEND_TEST(OSM_GeometryHandler, addUnnamedAreaFromWay);
  std::ofstream _ofsUnnamedAreas;
  boost::archive::binary_oarchive _oaUnnamedAreas;

  size_t _numNodes = 0;
  FRIEND_TEST(OSM_GeometryHandler, addNode);
  std::ofstream _ofsNodes;
  boost::archive::binary_oarchive _oaNodes;

  size_t _numWays = 0;
  FRIEND_TEST(OSM_GeometryHandler, addWay);
  std::ofstream _ofsWays;
  boost::archive::binary_oarchive _oaWays;

  size_t _dummyAreaCount = 0;
};

}  // namespace osm2rdf::osm

namespace boost::serialization {

template <class Archive>
void serialize(Archive& ar, osm2rdf::osm::SpatialNodeValue& v,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("id", std::get<0>(v));
  ar& boost::serialization::make_nvp("geom", std::get<1>(v));
}

template <class Archive>
void serialize(Archive& ar, osm2rdf::osm::SpatialWayValue& v,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("envelope", std::get<0>(v));
  ar& boost::serialization::make_nvp("id", std::get<1>(v));
  ar& boost::serialization::make_nvp("geom", std::get<2>(v));
  ar& boost::serialization::make_nvp("nodeIds", std::get<3>(v));
  ar& boost::serialization::make_nvp("boxes", std::get<4>(v));
  ar& boost::serialization::make_nvp("boxids", std::get<5>(v));
}

template <class Archive>
void serialize(Archive& ar, osm2rdf::osm::SpatialAreaValue& v,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("envelope", std::get<0>(v));
  ar& boost::serialization::make_nvp("id", std::get<1>(v));
  ar& boost::serialization::make_nvp("geom", std::get<2>(v));
  ar& boost::serialization::make_nvp("objId", std::get<3>(v));
  ar& boost::serialization::make_nvp("geomArea", std::get<4>(v));
  ar& boost::serialization::make_nvp("fromWay", std::get<5>(v));
  ar& boost::serialization::make_nvp("inner", std::get<6>(v));
  ar& boost::serialization::make_nvp("outer", std::get<7>(v));
  ar& boost::serialization::make_nvp("boxids", std::get<8>(v));
  ar& boost::serialization::make_nvp("cutouts", std::get<9>(v));
  ar& boost::serialization::make_nvp("convexhull", std::get<10>(v));
}

}  // namespace boost::serialization

#endif  // OSM2RDF_OSM_GEOMETRYHANDLER_H_
