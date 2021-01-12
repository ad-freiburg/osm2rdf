// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_BOX_H_
#define OSM2TTL_GEOMETRY_BOX_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/split_free.hpp"
#include "osm2ttl/geometry/Location.h"

namespace osm2ttl {
namespace geometry {
typedef boost::geometry::model::box<osm2ttl::geometry::Location> Box;
}  // namespace geometry
}  // namespace osm2ttl

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const osm2ttl::geometry::Box& m,
          [[maybe_unused]] const unsigned int version) {
  ar << boost::serialization::make_nvp("min_corner", m.min_corner());
  ar << boost::serialization::make_nvp("max_corner", m.max_corner());
}
template <class Archive>
void load(Archive& ar, osm2ttl::geometry::Box& m,
          [[maybe_unused]] const unsigned int version) {
  ar >> boost::serialization::make_nvp("min_corner", m.min_corner());
  ar >> boost::serialization::make_nvp("max_corner", m.max_corner());
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(osm2ttl::geometry::Box)
#endif  // OSM2TTL_GEOMETRY_BOX_H_
