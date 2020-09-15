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

typedef std::pair<osm2ttl::geometry::Box, uint64_t> SpatialValue;

template<typename W>
class GeometryHandler : public osmium::handler::Handler {
 public:
  GeometryHandler(const osm2ttl::config::Config& config,
                       osm2ttl::ttl::Writer<W>* writer);
  ~GeometryHandler();

  // Store area
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
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
  // Stacks
  boost::geometry::index::rtree<
      SpatialValue,
      boost::geometry::index::quadratic<16>> _spatialIndex;
  std::unordered_multimap<uint64_t, uint64_t> _locationRelationMap;
  std::unordered_multimap<uint64_t, uint64_t> _wayRelationMap;
  std::unordered_multimap<uint64_t, uint64_t> _wayLocationMap;
  double _xFactor = 1.5;
  double _yFactor = 0.75;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_GEOMETRYHANDLER_H_
