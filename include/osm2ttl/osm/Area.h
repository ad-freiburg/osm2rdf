// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREA_H_
#define OSM2TTL_OSM_AREA_H_

static const int Base10Base = 10;
#include <utility>
#include <vector>

#include "boost/geometry/geometries/geometries.hpp"
#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/osm/Box.h"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

namespace osm2ttl {
namespace osm {

struct Area {
  typedef double AreaType;

  Area();
  explicit Area(const osmium::Area& area);
  [[nodiscard]] uint64_t id() const noexcept;
  [[nodiscard]] uint64_t objId() const noexcept;

  [[nodiscard]] osm2ttl::geometry::Area geom() const noexcept;
  [[nodiscard]] AreaType geomArea() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] AreaType envelopeArea() const noexcept;
  [[nodiscard]] char tagAdministrationLevel() const noexcept;
  [[nodiscard]] bool fromWay() const noexcept;
  [[nodiscard]] bool hasName() const noexcept;

  bool operator==(const osm2ttl::osm::Area& other) const;
  bool operator<(const osm2ttl::osm::Area& other) const;

 protected:
  uint64_t _id;
  osmium::unsigned_object_id_type _objId;
  char _tagAdministrationLevel;
  bool _hasName;
  AreaType _geomArea;
  AreaType _envelopeArea;
  osm2ttl::geometry::Area _geom;
  osm2ttl::geometry::Box _envelope;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREA_H_
