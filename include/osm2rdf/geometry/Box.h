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

#ifndef OSM2RDF_GEOMETRY_BOX_H_
#define OSM2RDF_GEOMETRY_BOX_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/split_free.hpp"
#include "osm2rdf/geometry/Location.h"

namespace osm2rdf::geometry {
typedef boost::geometry::model::box<osm2rdf::geometry::Location> Box;
}  // namespace osm2rdf::geometry

inline bool operator==(const osm2rdf::geometry::Box& lhs,
                       const osm2rdf::geometry::Box& rhs) noexcept {
  return lhs.min_corner() == rhs.min_corner() &&
         lhs.max_corner() == rhs.max_corner();
}
inline bool operator!=(const osm2rdf::geometry::Box& lhs,
                       const osm2rdf::geometry::Box& rhs) noexcept {
  return !(lhs == rhs);
}

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2rdf::geometry::Box& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("min_corner", m.min_corner());
  ar& boost::serialization::make_nvp("max_corner", m.max_corner());
}
}  // namespace boost::serialization

#endif  // OSM2RDF_GEOMETRY_BOX_H_
