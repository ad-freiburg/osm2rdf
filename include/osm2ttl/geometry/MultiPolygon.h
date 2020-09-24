// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_MULTIPOLYGON_H_
#define OSM2TTL_GEOMETRY_MULTIPOLYGON_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "osm2ttl/geometry/Polygon.h"

namespace osm2ttl {
namespace geometry {
typedef boost::geometry::model::multi_polygon<osm2ttl::geometry::Polygon>
    MultiPolygon;
}  // namespace geometry
}  // namespace osm2ttl

#endif  // OSM2TTL_GEOMETRY_MULTIPOLYGON_H_
