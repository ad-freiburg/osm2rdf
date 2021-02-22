// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_LINESTRING_H_
#define OSM2TTL_GEOMETRY_LINESTRING_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2ttl/geometry/Location.h"

namespace osm2ttl::geometry {
typedef boost::geometry::model::linestring<osm2ttl::geometry::Location>
    Linestring;

bool operator==(const osm2ttl::geometry::Linestring& l1, const osm2ttl::geometry::Linestring& l2) {
  if (l1.size() != l2.size()) {
    return false;
  }
  for (size_t i = 0; i < l1.size(); ++i) {
    if (l1[i] != l2[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const osm2ttl::geometry::Linestring& l1, const osm2ttl::geometry::Linestring& l2) {
  return !(l1 == l2);
}
}  // namespace osm2ttl::geometry

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2ttl::geometry::Linestring& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp(
      "locations", static_cast<std::vector<osm2ttl::geometry::Location>&>(m));
}
}  // namespace boost::serialization

#endif  // OSM2TTL_GEOMETRY_LINESTRING_H_
