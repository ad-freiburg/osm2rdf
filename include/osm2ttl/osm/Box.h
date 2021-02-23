// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

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
