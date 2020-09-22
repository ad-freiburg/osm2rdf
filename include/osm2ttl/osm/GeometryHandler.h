// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_GEOMETRYHANDLER_H_
#define OSM2TTL_OSM_GEOMETRYHANDLER_H_

#include <unordered_map>
#include <utility>
#include <vector>

#include "osmium/handler.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/CacheFile.h"

#include "boost/geometry/index/rtree.hpp"

namespace osm2ttl {
namespace osm {

typedef std::pair<osm2ttl::geometry::Box, uint64_t> SpatialBoxValue;
typedef std::pair<osm2ttl::geometry::Location , uint64_t> SpatialPointValue;
typedef std::pair<osm2ttl::geometry::Box, std::pair<uint64_t, uint8_t>> SpatialValue;
typedef boost::geometry::index::rtree<SpatialBoxValue, boost::geometry::index::quadratic<16>> SpatialAreaIndex;
typedef boost::geometry::index::rtree<SpatialPointValue, boost::geometry::index::quadratic<16>> SpatialNodeIndex;
typedef boost::geometry::index::rtree<SpatialBoxValue, boost::geometry::index::quadratic<16>> SpatialWayIndex;
typedef boost::geometry::index::rtree<SpatialValue, boost::geometry::index::quadratic<16>> SpatialIndex;

template<typename W>
class GeometryHandler : public osmium::handler::Handler {
 public:
  GeometryHandler(const osm2ttl::config::Config& config,
                       osm2ttl::ttl::Writer<W>* writer);
  ~GeometryHandler();

  // Store data
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void way(const osmium::Way& way);
  // Calculate data
  void lookup();
  void prepareLookup();
 protected:
  bool _sorted = false;
  // Global config
  osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
  // Areas
  osm2ttl::util::CacheFile _areasFile;
  osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osm2ttl::osm::Area>
    _areas;
  // Ways
  osm2ttl::util::CacheFile _waysFile;
  osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osm2ttl::osm::Way>
      _ways;
  // Spatial Indices
  SpatialAreaIndex _spatialAreaIndex;
  SpatialNodeIndex _spatialNodeIndex;
  SpatialWayIndex _spatialWayIndex;
  SpatialIndex _spatialIndex;
  double _xFactor = 1.5;
  double _yFactor = 0.75;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_GEOMETRYHANDLER_H_
