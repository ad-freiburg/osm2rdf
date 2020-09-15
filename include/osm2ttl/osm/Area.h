// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREA_H_
#define OSM2TTL_OSM_AREA_H_

#include <vector>
#include <utility>

#include "boost/geometry/geometries/geometries.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/osm/Box.h"

namespace osm2ttl {
namespace osm {

struct Area {
  Area();
  explicit Area(const osmium::Area& area);
  uint64_t id() const noexcept;
  uint64_t objId() const noexcept;
  osm2ttl::geometry::Box envelope() const noexcept;
  char tagAdministrationLevel() const noexcept;
  bool fromWay() const noexcept;

  bool operator==(const osm2ttl::osm::Area& other) const;
  bool operator<(const osm2ttl::osm::Area& other) const;

  osm2ttl::geometry::Area geom() const;

 protected:
  uint64_t _id;
  osmium::unsigned_object_id_type _objId;
  char _tagAdministrationLevel;
  osm2ttl::geometry::Area _geom;
  osm2ttl::geometry::Box _envelope;
};

}  // namespace osm
}  // namespace osm2ttl

// Make area hashable for stl-container
namespace std {

template<>
struct hash<osm2ttl::osm::Area> {
  std::size_t operator()(const osm2ttl::osm::Area& area) const noexcept {
    return area.id();
  }
};

}  // namespace std

#endif  // OSM2TTL_OSM_AREA_H_
