// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_GEOMETRYHANDLER_H_
#define OSM2TTL_OSM_GEOMETRYHANDLER_H_

#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "boost/geometry/index/rtree.hpp"
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
#include "osmium/handler.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

namespace osm2ttl {
namespace osm {

typedef std::tuple<osm2ttl::geometry::Box, uint64_t, osm2ttl::geometry::Area,
                   uint64_t, osm2ttl::osm::Area::AreaType , bool>
    SpatialAreaValue;

typedef std::tuple<osm2ttl::geometry::Box, uint64_t, osm2ttl::geometry::Node>
    SpatialNodeValue;

typedef std::tuple<osm2ttl::geometry::Box, uint64_t, osm2ttl::geometry::Way,
                   std::vector<uint64_t>>
    SpatialWayValue;
typedef boost::geometry::index::rtree<SpatialAreaValue,
                                      boost::geometry::index::quadratic<16>>
    SpatialIndex;

typedef std::unordered_map<uint64_t, std::vector<uint64_t>> NodeData;

template <typename W>
class GeometryHandler : public osmium::handler::Handler {
 public:
  GeometryHandler(const osm2ttl::config::Config& config,
                  osm2ttl::ttl::Writer<W>* writer);

  // Store data
  void area(const osm2ttl::osm::Area& area);
  void node(const osm2ttl::osm::Node& node);
  void way(const osm2ttl::osm::Way& way);
  // Calculate data
  void calculateRelations();

 protected:
  // Stores named areas in r-tree, used for all other calculations.
  void prepareRTree();
  // Generate DAG for areas using prepared r-tree.
  void prepareDAG();
  // Calculate relations for each area, this dumps the generated DAG.
  void dumpNamedAreaRelations();
  // Calculate relations for each node.
  NodeData dumpNodeRelations();
  // Calculate relations for each way.
  void dumpRelationRelations(const osm2ttl::osm::NodeData& nodeData);
  // Calculate relations for each area, this dumps the generated DAG.
  void dumpUnnamedAreaRelations();
  std::string statisticLine(std::string_view function, std::string_view part,
                          std::string_view check, uint64_t outerId,
                          std::string_view outerType, uint64_t innerId,
                          std::string_view innerType,
                          std::chrono::nanoseconds durationNS,
                          bool result);
  // Global config
  osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
  // Statistics
  osm2ttl::util::Output _statistics;
  // Store areas as r-tree
  SpatialIndex spatialIndex;
  // Store dag
  osm2ttl::util::DirectedGraph directedAreaGraph;
  // Spatial Data
  std::vector<SpatialAreaValue> _spatialStorageArea;
  std::vector<SpatialAreaValue> _spatialStorageUnnamedArea;
  std::unordered_map<uint64_t, uint64_t> _areaData;
  std::vector<SpatialNodeValue> _spatialStorageNode;
  std::vector<SpatialWayValue> _spatialStorageWay;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_GEOMETRYHANDLER_H_
