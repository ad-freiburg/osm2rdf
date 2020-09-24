// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_BOX_H_
#define OSM2TTL_GEOMETRY_BOX_H_

#include "boost/geometry/geometries/geometries.hpp"

#include "osm2ttl/geometry/Location.h"

namespace osm2ttl {
namespace geometry {
typedef boost::geometry::model::box<osm2ttl::geometry::Location> Box;
}  // namespace geometry
}  // namespace osm2ttl

#endif  // OSM2TTL_GEOMETRY_BOX_H_
