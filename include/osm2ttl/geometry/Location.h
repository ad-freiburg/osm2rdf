// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_LOCATION_H_
#define OSM2TTL_GEOMETRY_LOCATION_H_

#include "boost/geometry/geometries/point_xy.hpp"
#include "osm2ttl/geometry/Global.h"

namespace osm2ttl {
namespace geometry {
typedef boost::geometry::model::d2::point_xy<osm2ttl::geometry::location_coordinate_t> Location;
}  // namespace geometry
}  // namespace osm2ttl

#endif  // OSM2TTL_GEOMETRY_LOCATION_H_
