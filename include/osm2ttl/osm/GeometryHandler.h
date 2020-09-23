// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_GEOMETRYHANDLER_H_
#define OSM2TTL_OSM_GEOMETRYHANDLER_H_

#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "boost/geometry/index/rtree.hpp"

#include "osmium/handler.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/geometry/Linestring.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/CacheFile.h"


namespace osm2ttl {
namespace osm {

typedef std::pair<osm2ttl::geometry::Box, std::pair<uint64_t, std::variant<osm2ttl::geometry::Location, osm2ttl::geometry::Linestring, osm2ttl::geometry::Area>>> SpatialValue;
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
  // Spatial Index
  std::vector<osm2ttl::osm::Area> _containingAreas;
  std::vector<SpatialValue> _spatialStorage;
  SpatialIndex _spatialIndex;
  double _xFactor = 1.5;
  double _yFactor = 0.75;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_GEOMETRYHANDLER_H_
