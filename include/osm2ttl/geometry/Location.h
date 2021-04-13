// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2TTL_GEOMETRY_LOCATION_H_
#define OSM2TTL_GEOMETRY_LOCATION_H_

#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/split_free.hpp"
#include "osm2ttl/geometry/Global.h"

namespace osm2ttl::geometry {
typedef boost::geometry::model::d2::point_xy<
    osm2ttl::geometry::location_coordinate_t>
    Location;

}  // namespace osm2ttl::geometry

inline bool operator==(const osm2ttl::geometry::Location& lhs,
                       const osm2ttl::geometry::Location& rhs) noexcept {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

inline bool operator!=(const osm2ttl::geometry::Location& lhs,
                       const osm2ttl::geometry::Location& rhs) noexcept {
  return !(lhs == rhs);
}

namespace boost::serialization {
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
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(osm2ttl::geometry::Location)
#endif  // OSM2TTL_GEOMETRY_LOCATION_H_
