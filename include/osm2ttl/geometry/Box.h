// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_BOX_H_
#define OSM2TTL_GEOMETRY_BOX_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/split_free.hpp"
#include "osm2ttl/geometry/Location.h"

namespace osm2ttl::geometry {
typedef boost::geometry::model::box<osm2ttl::geometry::Location> Box;
}  // namespace osm2ttl::geometry

inline bool operator==(const osm2ttl::geometry::Box& lhs,
                       const osm2ttl::geometry::Box& rhs) noexcept {
  return lhs.min_corner() == rhs.min_corner() &&
         lhs.max_corner() == rhs.max_corner();
}
inline bool operator!=(const osm2ttl::geometry::Box& lhs,
                       const osm2ttl::geometry::Box& rhs) noexcept {
  return !(lhs == rhs);
}

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2ttl::geometry::Box& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("min_corner", m.min_corner());
  ar& boost::serialization::make_nvp("max_corner", m.max_corner());
}
}  // namespace boost::serialization

#endif  // OSM2TTL_GEOMETRY_BOX_H_
