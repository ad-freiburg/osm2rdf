// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_POLYGON_H_
#define OSM2TTL_GEOMETRY_POLYGON_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/geometry/Ring.h"

namespace osm2ttl::geometry {
typedef boost::geometry::model::polygon<osm2ttl::geometry::Location> Polygon;

inline bool operator==(const osm2ttl::geometry::Polygon& lhs,
                       const osm2ttl::geometry::Polygon& rhs) {
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

inline bool operator!=(const osm2ttl::geometry::Polygon& lhs,
                       const osm2ttl::geometry::Polygon& rhs) {
  return !(lhs == rhs);
}
}  // namespace osm2ttl::geometry

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2ttl::geometry::Polygon& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("outer", m.outer());
  ar& boost::serialization::make_nvp("inners", m.inners());
}
}  // namespace boost::serialization

#endif  // OSM2TTL_GEOMETRY_POLYGON_H_
