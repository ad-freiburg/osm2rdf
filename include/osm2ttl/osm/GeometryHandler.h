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

#ifndef OSM2TTL_OSM_GEOMETRYHANDLER_H_
#define OSM2TTL_OSM_GEOMETRYHANDLER_H_

#include <unordered_map>
#include <utility>
#include <vector>

#include "boost/archive/binary_oarchive.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "gtest/gtest_prod.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/geometry/Node.h"
#include "osm2ttl/geometry/Way.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/CacheFile.h"
#include "osm2ttl/util/DirectedGraph.h"
#include "osm2ttl/util/Output.h"

namespace osm2ttl::osm {

// Area: envelope, id, geometry, osm id, area, fromWay
typedef std::tuple<osm2ttl::geometry::Box, osm2ttl::osm::Area::id_t,
                   osm2ttl::geometry::Area, osm2ttl::osm::Area::id_t,
                   osm2ttl::geometry::area_result_t, bool>
    SpatialAreaValue;

typedef std::vector<SpatialAreaValue> SpatialAreaVector;

// Node: envelope, osm  id, geometry
typedef std::tuple<osm2ttl::geometry::Box, osm2ttl::osm::Node::id_t,
                   osm2ttl::geometry::Node>
    SpatialNodeValue;
typedef std::vector<SpatialNodeValue> SpatialNodeVector;

typedef std::vector<osm2ttl::osm::Node::id_t> WayNodeList;

// Way: envelope, osm id, geometry, node list
typedef std::tuple<osm2ttl::geometry::Box, osm2ttl::osm::Way::id_t,
                   osm2ttl::geometry::Way, WayNodeList>
    SpatialWayValue;
typedef std::vector<SpatialWayValue> SpatialWayVector;

typedef boost::geometry::index::rtree<SpatialAreaValue,
                                      boost::geometry::index::quadratic<16>>
    SpatialIndex;

// node osm id -> area ids (not osm id)
typedef std::unordered_map<osm2ttl::osm::Node::id_t,
                           std::vector<osm2ttl::osm::Area::id_t>>
    NodesContainedInAreasData;

template <typename W>
class GeometryHandler {
 public:
  GeometryHandler(const osm2ttl::config::Config& config,
                  osm2ttl::ttl::Writer<W>* writer);
  ~GeometryHandler();

  // Add data
  void area(const osm2ttl::osm::Area& area);
  void node(const osm2ttl::osm::Node& node);
  void way(const osm2ttl::osm::Way& way);
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
      const osm2ttl::osm::NodesContainedInAreasData& nodeData);
  FRIEND_TEST(OSM_GeometryHandler, noWayGeometricRelations);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsEmpty1);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsEmpty2);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleIntersects);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleContains);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleIntersectsWithNodeInfo);
  FRIEND_TEST(OSM_GeometryHandler, dumpWayRelationsSimpleContainsWithNodeInfo);

  // Write a statistic line for a given spatial check
  [[nodiscard]] std::string statisticLine(std::string_view function, std::string_view part,
                            std::string_view check, uint64_t outerId,
                            std::string_view outerType, uint64_t innerId,
                            std::string_view innerType,
                            std::chrono::nanoseconds durationNS, bool result);
  FRIEND_TEST(OSM_GeometryHandler, statisticLine);

  // Global config
  osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
  // Statistics
  osm2ttl::util::Output _statistics;
  // Store areas as r-tree
  SpatialIndex _spatialIndex;
  // Store dag
  osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t> _directedAreaGraph;
  // Spatial Data
  SpatialAreaVector _spatialStorageArea;
  std::unordered_map<osm2ttl::osm::Area::id_t, uint64_t>
      _spatialStorageAreaIndex;
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
};

}  // namespace osm2ttl::osm

namespace boost::serialization {

template <class Archive>
void serialize(Archive& ar, osm2ttl::osm::SpatialNodeValue& v,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("envelope", std::get<0>(v));
  ar& boost::serialization::make_nvp("id", std::get<1>(v));
  ar& boost::serialization::make_nvp("geom", std::get<2>(v));
}

template <class Archive>
void serialize(Archive& ar, osm2ttl::osm::SpatialWayValue& v,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("envelope", std::get<0>(v));
  ar& boost::serialization::make_nvp("id", std::get<1>(v));
  ar& boost::serialization::make_nvp("geom", std::get<2>(v));
  ar& boost::serialization::make_nvp("nodeIds", std::get<3>(v));
}

template <class Archive>
void serialize(Archive& ar, osm2ttl::osm::SpatialAreaValue& v,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("envelope", std::get<0>(v));
  ar& boost::serialization::make_nvp("id", std::get<1>(v));
  ar& boost::serialization::make_nvp("geom", std::get<2>(v));
  ar& boost::serialization::make_nvp("objId", std::get<3>(v));
  ar& boost::serialization::make_nvp("geomArea", std::get<4>(v));
  ar& boost::serialization::make_nvp("fromWay", std::get<5>(v));
}

}  // namespace boost::serialization

#endif  // OSM2TTL_OSM_GEOMETRYHANDLER_H_
