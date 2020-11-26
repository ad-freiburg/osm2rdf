// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_GLOBAL_H
#define OSM2TTL_GEOMETRY_GLOBAL_H

#include "boost/geometry/strategies/cartesian/area.hpp"

namespace osm2ttl {
namespace geometry {
typedef double location_coordinate_t;
typedef double area_result_t;
typedef boost::geometry::strategy::area::cartesian<area_result_t> area_strategy;
}
}

#endif  // OSM2TTL_GEOMETRY_GLOBAL_H
