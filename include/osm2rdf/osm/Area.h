// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2RDF_OSM_AREA_H_
#define OSM2RDF_OSM_AREA_H_

#include <utility>
#include <vector>

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Global.h"
#include "osm2rdf/osm/Box.h"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

namespace osm2rdf::osm {

struct Area {
  typedef uint32_t id_t;

  Area();
  explicit Area(const osmium::Area& area);
  // Osmium id.
  [[nodiscard]] id_t id() const noexcept;
  // OSM id.
  [[nodiscard]] id_t objId() const noexcept;

  // Return the geometry.
  [[nodiscard]] const osm2rdf::geometry::Area& geom() const noexcept;
  // Return area of the geometry.
  [[nodiscard]] osm2rdf::geometry::area_result_t geomArea() const noexcept;
  // Return the envelope.
  [[nodiscard]] const osm2rdf::geometry::Box& envelope() const noexcept;
  // Return the are of the envelope.
  [[nodiscard]] osm2rdf::geometry::area_result_t envelopeArea() const noexcept;
  // Return if this area is created from a way.
  [[nodiscard]] bool fromWay() const noexcept;
  // Return if this area has a name.
  [[nodiscard]] bool hasName() const noexcept;
  // finalize geometries
  void finalize() noexcept;

  bool operator==(const osm2rdf::osm::Area& other) const noexcept;
  bool operator!=(const osm2rdf::osm::Area& other) const noexcept;

 protected:
  // The osmium id
  id_t _id;
  // The OSM id
  id_t _objId;
  bool _hasName;
  osm2rdf::geometry::area_result_t _geomArea;
  osm2rdf::geometry::area_result_t _envelopeArea;
  osm2rdf::geometry::Area _geom;
  osm2rdf::geometry::Box _envelope;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_objId", _objId);
    ar& boost::serialization::make_nvp("_hasName", _hasName);
    ar& boost::serialization::make_nvp("_geomArea", _geomArea);
    ar& boost::serialization::make_nvp("_envelopeArea", _envelopeArea);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_envelope", _envelope);
  }
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_AREA_H_
