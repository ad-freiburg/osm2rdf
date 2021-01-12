// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_LOCATION_H_
#define OSM2TTL_GEOMETRY_LOCATION_H_

#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/split_free.hpp"
#include "osm2ttl/geometry/Global.h"

namespace osm2ttl {
namespace geometry {
typedef boost::geometry::model::d2::point_xy<
    osm2ttl::geometry::location_coordinate_t>
    Location;
}  // namespace geometry
}  // namespace osm2ttl

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const osm2ttl::geometry::Location& m,
          [[maybe_unused]] const unsigned int version) {
  ar << boost::serialization::make_nvp("x", m.x());
  ar << boost::serialization::make_nvp("y", m.y());
}
template <class Archive>
void load(Archive& ar, osm2ttl::geometry::Location& m,
          [[maybe_unused]] const unsigned int version) {
  osm2ttl::geometry::location_coordinate_t x;
  osm2ttl::geometry::location_coordinate_t y;
  ar >> boost::serialization::make_nvp("x", x);
  ar >> boost::serialization::make_nvp("y", y);
  m.x(x);
  m.y(y);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(osm2ttl::geometry::Location)
#endif  // OSM2TTL_GEOMETRY_LOCATION_H_
