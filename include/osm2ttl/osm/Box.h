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

#ifndef OSM2TTL_OSM_BOX_H_
#define OSM2TTL_OSM_BOX_H_

#include "boost/serialization/nvp.hpp"
#include "osm2ttl/geometry/Box.h"

namespace osm2ttl::osm {

class Box {
 public:
  Box();
  explicit Box(const osm2ttl::geometry::Box& box);
  [[nodiscard]] osm2ttl::geometry::Box geom() const;

  bool operator==(const osm2ttl::osm::Box& other) const noexcept;
  bool operator!=(const osm2ttl::osm::Box& other) const noexcept;

 protected:
  osm2ttl::geometry::Box _geom{};

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_geom", _geom);
  }
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_BOX_H_
