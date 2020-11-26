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
  typedef uint32_t id_t;
  typedef double area_t;

  Area();
  explicit Area(const osmium::Area& area);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] id_t objId() const noexcept;

  [[nodiscard]] osm2ttl::geometry::Area geom() const noexcept;
  [[nodiscard]] area_t geomArea() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] area_t envelopeArea() const noexcept;
  [[nodiscard]] char tagAdministrationLevel() const noexcept;
  [[nodiscard]] bool fromWay() const noexcept;
  [[nodiscard]] bool hasName() const noexcept;

  bool operator==(const osm2ttl::osm::Area& other) const;
  bool operator<(const osm2ttl::osm::Area& other) const;

 protected:
  id_t _id;
  id_t _objId;
  char _tagAdministrationLevel;
  bool _hasName;
  area_t _geomArea;
  area_t _envelopeArea;
  osm2ttl::geometry::Area _geom;
  osm2ttl::geometry::Box _envelope;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREA_H_
