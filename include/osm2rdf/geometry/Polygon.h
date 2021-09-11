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

#ifndef OSM2RDF_GEOMETRY_POLYGON_H_
#define OSM2RDF_GEOMETRY_POLYGON_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/geometry/Ring.h"

namespace osm2rdf::geometry {
typedef boost::geometry::model::polygon<osm2rdf::geometry::Location> Polygon;
}  // namespace osm2rdf::geometry

inline bool operator==(const osm2rdf::geometry::Polygon& lhs,
                       const osm2rdf::geometry::Polygon& rhs) noexcept {
  if (lhs.outer() != rhs.outer()) {
    return false;
  }
  if (lhs.inners().size() != rhs.inners().size()) {
    return false;
  }
  for (size_t i = 0; i < lhs.inners().size(); ++i) {
    if (lhs.inners()[i] != rhs.inners()[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const osm2rdf::geometry::Polygon& lhs,
                       const osm2rdf::geometry::Polygon& rhs) noexcept {
  return !(lhs == rhs);
}

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2rdf::geometry::Polygon& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("outer", m.outer());
  ar& boost::serialization::make_nvp("inners", m.inners());
}
}  // namespace boost::serialization

#endif  // OSM2RDF_GEOMETRY_POLYGON_H_
