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
#include "osm2ttl/util/CacheFile.h"

namespace osm2ttl {
namespace osm {

template<typename W>
class AreaHandler : public osmium::handler::Handler {
 public:
  AreaHandler(const osm2ttl::config::Config& config,
                       osm2ttl::ttl::Writer<W>* writer);
  ~AreaHandler();

  // Store area
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
  void sort();
  constexpr uint16_t stackIndex(uint8_t x, uint8_t y) const;
  osm2ttl::geometry::Polygon regionForIndex(uint8_t x, uint8_t y) const;
  std::pair<uint8_t, uint8_t> reducedCoordinates(double x, double y) const;
  std::pair<uint8_t, uint8_t> reducedCoordinates(const osm2ttl::geometry::Location& location) const;
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
  std::vector<std::vector<std::pair<bool, osm2ttl::osm::Area*>>> _stacks;
  std::unordered_multimap<uint64_t, uint64_t> _locationRelationMap;
  std::unordered_multimap<uint64_t, uint64_t> _wayRelationMap;
  std::unordered_multimap<uint64_t, uint64_t> _wayLocationMap;
  double _xFactor = 1.5;
  double _yFactor = 0.75;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREAHANDLER_H_
