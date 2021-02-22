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

bool operator==(const osm2ttl::geometry::Box& b1,
                const osm2ttl::geometry::Box& b2) {
  return b1.min_corner() == b2.min_corner() &&
         b1.max_corner() == b2.max_corner();
}
bool operator!=(const osm2ttl::geometry::Box& b1,
                const osm2ttl::geometry::Box& b2) {
  return !(b1 == b2);
}
}  // namespace osm2ttl::geometry

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2ttl::geometry::Box& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp("min_corner", m.min_corner());
  ar& boost::serialization::make_nvp("max_corner", m.max_corner());
}
}  // namespace boost::serialization

#endif  // OSM2TTL_GEOMETRY_BOX_H_
