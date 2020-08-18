// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_LOCATION_H_
#define OSM2TTL_GEOMETRY_LOCATION_H_

#include "boost/geometry/geometries/geometries.hpp"

namespace osm2ttl {
namespace geometry {
  typedef boost::geometry::model::point<
    double, 2, boost::geometry::cs::cartesian> Location;
}  // namespace geometry
}  // namespace osm2ttl

#endif  // OSM2TTL_GEOMETRY_LOCATION_H_
