// Copyright 2022, University of Freiburg
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

#ifndef OSM2RDF_GEOMETRY_GEOMETRYCOLLECTION_H
#define OSM2RDF_GEOMETRY_GEOMETRYCOLLECTION_H

#include <boost/geometry/algorithms/equals.hpp>
#include "boost/geometry/geometries/geometries.hpp"
#include "boost/variant.hpp"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Node.h"
#include "osm2rdf/geometry/Way.h"

#if BOOST_VERSION >= 107700

namespace osm2rdf::geometry {
typedef boost::variant<osm2rdf::geometry::Node, osm2rdf::geometry::Way,
                       osm2rdf::geometry::Area>
    RelationGeometryParts_t;
typedef boost::geometry::model::geometry_collection<RelationGeometryParts_t>
    Relation;
}  // namespace osm2rdf::geometry

inline bool operator==(const osm2rdf::geometry::Relation& lhs,
                       const osm2rdf::geometry::Relation& rhs) noexcept {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (size_t i = 0; i < lhs.size(); ++i) {
    if (!boost::geometry::equals(lhs[i], rhs[i])) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const osm2rdf::geometry::Relation& lhs,
                       const osm2rdf::geometry::Relation& rhs) noexcept {
  return !(lhs == rhs);
}

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2rdf::geometry::RelationGeometryParts_t& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp(
      "data",
      static_cast<osm2rdf::geometry::RelationGeometryParts_t&>(m));
}

template <class Archive>
void serialize(Archive& ar, osm2rdf::geometry::Relation& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp(
      "parts",
      static_cast<std::vector<osm2rdf::geometry::RelationGeometryParts_t>&>(m));
}
}  // namespace boost::serialization

#endif  // BOOST_VERSION >= 107700

#endif  // OSM2RDF_GEOMETRY_GEOMETRYCOLLECTION_H
