// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREA_H_
#define OSM2TTL_OSM_AREA_H_

#include <vector>
#include <utility>

#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

#include "osm2ttl/osm/Ring.h"

namespace osm2ttl {
namespace osm {

struct Area {
  Area();
  explicit Area(const osmium::Area& area);
  uint64_t id() const noexcept;
  uint64_t objId() const noexcept;
  osmium::Box bbox() const noexcept;
  osmium::Location centroid() const noexcept;
  char tagAdministrationLevel() const noexcept;
  bool fromWay() const noexcept;

  std::vector<osm2ttl::osm::OuterRing> rings() const noexcept;

  double area() const noexcept;
  bool contains(const Area& other) const noexcept;
  bool intersects(const Area& other) const noexcept;
  // Vague functions
  double vagueArea() const noexcept;
  bool vagueContains(const Area& other) const noexcept;
  bool vagueIntersects(const Area& other) const noexcept;

  bool operator==(const osm2ttl::osm::Area& other) const;
  bool operator<(const osm2ttl::osm::Area& other) const;

 protected:
  uint64_t _id;
  osmium::unsigned_object_id_type _objId;
  char _tagAdministrationLevel;
  std::vector<OuterRing> _rings;
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
