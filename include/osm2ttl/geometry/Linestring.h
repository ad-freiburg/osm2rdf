// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_LINESTRING_H_
#define OSM2TTL_GEOMETRY_LINESTRING_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "osm2ttl/geometry/Location.h"

namespace osm2ttl {
namespace geometry {
typedef boost::geometry::model::linestring<osm2ttl::geometry::Location>
    Linestring;
}  // namespace geometry
}  // namespace osm2ttl

#endif  // OSM2TTL_GEOMETRY_LINESTRING_H_
