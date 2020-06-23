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
#include "osm2ttl/ttl/Writer.h"

namespace osm2ttl {
namespace osm {

class AreaHandler : public osmium::handler::Handler {
 public:
  AreaHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer* writer);

  // Store area
  void area(const osmium::Area& area);
  osm2ttl::osm::Area lookup(uint64_t id) const;
  void sort();
 protected:
  // helper
  std::vector<std::pair<size_t, size_t>> stacksForArea(
    const osm2ttl::osm::Area& area);
  // Global config
  osm2ttl::config::Config _config;
  // Triple writer
  osm2ttl::ttl::Writer* _writer;
  // Areas
  std::unordered_map<uint64_t, osm2ttl::osm::Area> _areas;
  // Stacks
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREAHANDLER_H_
