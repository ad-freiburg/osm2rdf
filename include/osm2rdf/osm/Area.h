// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#include "osm2rdf/osm/Box.h"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"
#include "util/geo/Geo.h"

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
  [[nodiscard]] const ::util::geo::DMultiPolygon& geom() const noexcept;
  // Return area of the geometry.
  [[nodiscard]] double geomArea() const noexcept;
  // Return the envelope.
  [[nodiscard]] const ::util::geo::DBox& envelope() const noexcept;
  // Return the area of the envelope.
  [[nodiscard]] double envelopeArea() const noexcept;
  // Return the convex hull of the area.
  [[nodiscard]] const ::util::geo::DPolygon& convexHull() const noexcept;
  // Return the oriented bounding box of the area.
  [[nodiscard]] const ::util::geo::DPolygon& orientedBoundingBox() const noexcept;
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
  bool _hasName = false;
  double _geomArea = 0;
  double _envelopeArea = 0;
  ::util::geo::DMultiPolygon _geom;
  ::util::geo::DBox _envelope;
  ::util::geo::DPolygon _convexHull;
  ::util::geo::DPolygon _obb;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_AREA_H_
