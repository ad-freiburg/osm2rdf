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

#ifndef OSM2RDF_OSM_BOX_H_
#define OSM2RDF_OSM_BOX_H_

#include "util/geo/Geo.h"

namespace osm2rdf::osm {

class Box {
 public:
  Box();
  explicit Box(const ::util::geo::DBox& box);
  [[nodiscard]] const ::util::geo::DBox geom() const;
  [[nodiscard]] const ::util::geo::DPolygon convexHull() const noexcept;
  [[nodiscard]] const ::util::geo::DPolygon orientedBoundingBox() const noexcept;
  [[nodiscard]] const ::util::geo::DPoint centroid() const noexcept;

  bool operator==(const osm2rdf::osm::Box& other) const noexcept;
  bool operator!=(const osm2rdf::osm::Box& other) const noexcept;

 protected:
  ::util::geo::DBox _geom{};
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_BOX_H_
