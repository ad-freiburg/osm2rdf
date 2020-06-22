// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREAHANDLER_H_
#define OSM2TTL_OSM_AREAHANDLER_H_

#include <unordered_map>

#include "osmium/handler.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"

namespace osm2ttl {
namespace osm {

class AreaHandler : public osmium::handler::Handler {
 public:
  AreaHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer* writer,
              osmium::handler::NodeLocationsForWays<
      osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>* nodeLocations);

  // Store each area
  void area(const osmium::Area& area);
  // Calculate all intersection / containment relations for all areas
  void calculate();
 protected:
  // Global config
  osm2ttl::config::Config _config;
  // Triple writer
  osm2ttl::ttl::Writer* _writer;
  std::unordered_map<uint64_t, osm2ttl::osm::Area> _areas;
  uint64_t _areaCount;

  osmium::handler::NodeLocationsForWays<
    osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>* _nodeLocations;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREAHANDLER_H_
