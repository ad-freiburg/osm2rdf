// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2TTL_OSM_AREA_H_
#define OSM2TTL_OSM_AREA_H_

static const int Base10Base = 10;
#include <utility>
#include <vector>

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/geometry/Global.h"
#include "osm2ttl/osm/Box.h"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

namespace osm2ttl::osm {

struct Area {
  typedef uint32_t id_t;

  Area();
  explicit Area(const osmium::Area& area);
  // Osmium id.
  [[nodiscard]] id_t id() const noexcept;
  // OSM id.
  [[nodiscard]] id_t objId() const noexcept;

  // Return the geometry.
  [[nodiscard]] osm2ttl::geometry::Area geom() const noexcept;
  // Return area of the geometry.
  [[nodiscard]] osm2ttl::geometry::area_result_t geomArea() const noexcept;
  // Return the envelope.
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  // Return the are of the envelope.
  [[nodiscard]] osm2ttl::geometry::area_result_t envelopeArea() const noexcept;
  // Return the administration level, 0 otherwise.
  [[nodiscard]] char tagAdministrationLevel() const noexcept;
  // Return if this area is created from a way.
  [[nodiscard]] bool fromWay() const noexcept;
  // Return if this area has a name.
  [[nodiscard]] bool hasName() const noexcept;

  bool operator==(const osm2ttl::osm::Area& other) const noexcept;
  bool operator!=(const osm2ttl::osm::Area& other) const noexcept;
  bool operator<(const osm2ttl::osm::Area& other) const noexcept;

 protected:
  // The osmium id
  id_t _id;
  // The OSM id
  id_t _objId;
  char _tagAdministrationLevel;
  bool _hasName;
  osm2ttl::geometry::area_result_t _geomArea;
  osm2ttl::geometry::area_result_t _envelopeArea;
  osm2ttl::geometry::Area _geom;
  osm2ttl::geometry::Box _envelope;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_objId", _objId);
    ar& boost::serialization::make_nvp("_tagAdministrationLevel",
                                       _tagAdministrationLevel);
    ar& boost::serialization::make_nvp("_hasName", _hasName);
    ar& boost::serialization::make_nvp("_geomArea", _geomArea);
    ar& boost::serialization::make_nvp("_envelopeArea", _envelopeArea);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_envelope", _envelope);
  }
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_AREA_H_
