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

bool operator==(const osm2ttl::geometry::Polygon& l1,
                const osm2ttl::geometry::Polygon& l2) {
  if (l1.outer() != l2.outer()) {
    return false;
  }
  if (l1.inners().size() != l2.inners().size()) {
    return false;
  }
  for (size_t i = 0; i < l1.inners().size(); ++i) {
    if (l1.inners()[i] != l2.inners()[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const osm2ttl::geometry::Polygon& l1,
                const osm2ttl::geometry::Polygon& l2) {
  return !(l1 == l2);
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
