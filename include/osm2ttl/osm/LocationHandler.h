// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_LOCATIONHANDLER_H_
#define OSM2TTL_OSM_LOCATIONHANDLER_H_

#include "osm2ttl/config/Config.h"
#include "osm2ttl/util/CacheFile.h"
#include "osmium/handler.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/flex_mem.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/types.hpp"
#include "osmium/osm/way.hpp"

namespace osm2ttl::osm {

// Partially based on osmium::handler::ObjectRelations
class LocationHandler : public osmium::handler::Handler {
 public:
  virtual ~LocationHandler() {}
  virtual void node(const osmium::Node& node) = 0;
  virtual void way(osmium::Way& way) = 0;  // NOLINT
  // Helper creating the correct instance.
  static LocationHandler* create(const osm2ttl::config::Config& config);
};

template <typename T>
class LocationHandlerImpl : public LocationHandler {
 public:
  explicit LocationHandlerImpl(const osm2ttl::config::Config& config);
  void node(const osmium::Node& node);
  void way(osmium::Way& way);  // NOLINT
 protected:
  T _index;
  osmium::handler::NodeLocationsForWays<T> _handler;
};

template <>
class LocationHandlerImpl<osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::Location>>
    : public LocationHandler {
 public:
  explicit LocationHandlerImpl(const osm2ttl::config::Config& config);
  void node(const osmium::Node& node);
  void way(osmium::Way& way);  // NOLINT
 protected:
  osm2ttl::util::CacheFile _cacheFile;
  osmium::index::map::SparseFileArray<osmium::unsigned_object_id_type,
                                      osmium::Location>
      _index;
  osmium::handler::NodeLocationsForWays<osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>
      _handler;
};

using LocationHandlerRAM = LocationHandlerImpl<osmium::index::map::FlexMem<
    osmium::unsigned_object_id_type, osmium::Location>>;
using LocationHandlerFS =
    LocationHandlerImpl<osmium::index::map::SparseFileArray<
        osmium::unsigned_object_id_type, osmium::Location>>;

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_LOCATIONHANDLER_H_
