// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREAHANDLER_H_
#define OSM2TTL_OSM_AREAHANDLER_H_

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
#include "osm2ttl/osm/CacheFile.h"
#include "osm2ttl/ttl/Writer.h"

namespace osm2ttl {
namespace osm {

class AreaHandler : public osmium::handler::Handler {
 public:
  AreaHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer* writer);
  ~AreaHandler();

  // Store area
  void area(const osmium::Area& area);
  void sort();
 protected:
  // Global config
  osm2ttl::config::Config _config;
  // Triple writer
  osm2ttl::ttl::Writer* _writer;
  // Areas
  osm2ttl::osm::CacheFile _areasFile;
  osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osm2ttl::osm::Area>
    _areas;
  // Stacks
  std::vector<osmium::unsigned_object_id_type> _stacks;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREAHANDLER_H_
