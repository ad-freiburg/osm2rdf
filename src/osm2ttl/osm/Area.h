// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREA_H_
#define OSM2TTL_OSM_AREA_H_

#include <vector>
#include <utility>

#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

#include "osm2ttl/osm/Ring.h"

namespace osm2ttl {
namespace osm {

struct Area {
  Area(uint64_t id,
       const osmium::Area& area,
       osmium::handler::NodeLocationsForWays<
         osmium::index::map::SparseFileArray<
           osmium::unsigned_object_id_type, osmium::Location>>* nodeLocations);
  uint64_t id() const;
  uint64_t objId() const;
  osmium::Box bbox() const;
  char tagAdministrationLevel() const;

  std::vector<osm2ttl::osm::OuterRing> rings();

  double area() const;
  bool contains(const Area& other) const;
  bool intersects(const Area& other) const;
  // Vague functions
  double vagueArea() const;
  bool vagueContains(const Area& other) const;
  bool vagueIntersects(const Area& other) const;

 protected:
  double area(const osm2ttl::osm::Ring& ring) const;

  osmium::handler::NodeLocationsForWays<
      osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>* _nodeLocations;

  uint64_t _id;
  osmium::Box _box;
  osmium::unsigned_object_id_type _objId;
  char _tagAdministrationLevel;
  std::vector<OuterRing> _rings;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREA_H_
