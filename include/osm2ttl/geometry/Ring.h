// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_RING_H_
#define OSM2TTL_GEOMETRY_RING_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2ttl/geometry/Location.h"

namespace osm2ttl::geometry {
typedef boost::geometry::model::ring<osm2ttl::geometry::Location> Ring;

inline bool operator==(const osm2ttl::geometry::Ring& lhs,
                       const osm2ttl::geometry::Ring& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (size_t i = 0; i < lhs.size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const osm2ttl::geometry::Ring& lhs,
                       const osm2ttl::geometry::Ring& rhs) {
  return !(lhs == rhs);
}
}  // namespace osm2ttl::geometry

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2ttl::geometry::Ring& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp(
      "locations", static_cast<std::vector<osm2ttl::geometry::Location>&>(m));
}
}  // namespace boost::serialization

#endif  // OSM2TTL_GEOMETRY_POLYGON_H_
