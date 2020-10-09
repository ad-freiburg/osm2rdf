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
                   bool, double>
    SpatialAreaValue;

typedef std::tuple<osm2ttl::geometry::Box, uint64_t, osm2ttl::geometry::Node>
    SpatialNodeValue;

typedef std::tuple<osm2ttl::geometry::Box, uint64_t, osm2ttl::geometry::Way>
    SpatialWayValue;
typedef boost::geometry::index::rtree<SpatialAreaValue,
                                      boost::geometry::index::quadratic<16>>
    SpatialIndex;

template <typename W>
class GeometryHandler : public osmium::handler::Handler {
 public:
  GeometryHandler(const osm2ttl::config::Config& config,
                  osm2ttl::ttl::Writer<W>* writer);
  ~GeometryHandler() = default;

  // Store data
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void way(const osmium::Way& way);
  // Calculate data
  void lookup();

 protected:
  // Global config
  osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
  // Spatial Index
  std::vector<SpatialAreaValue> _spatialStorageArea;
  std::vector<SpatialNodeValue> _spatialStorageNode;
  std::vector<SpatialWayValue> _spatialStorageWay;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_GEOMETRYHANDLER_H_
