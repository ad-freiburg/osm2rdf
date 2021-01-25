// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_GEOMETRY_MULTIPOLYGON_H_
#define OSM2TTL_GEOMETRY_MULTIPOLYGON_H_

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2ttl/geometry/Polygon.h"

namespace osm2ttl::geometry {
typedef boost::geometry::model::multi_polygon<osm2ttl::geometry::Polygon>
    MultiPolygon;
}  // namespace osm2ttl::geometry

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, osm2ttl::geometry::MultiPolygon& m,
               [[maybe_unused]] const unsigned int version) {
  ar& boost::serialization::make_nvp(
      "polygons", static_cast<std::vector<osm2ttl::geometry::Polygon>&>(m));
}
}  // namespace boost::serialization

#endif  // OSM2TTL_GEOMETRY_MULTIPOLYGON_H_
