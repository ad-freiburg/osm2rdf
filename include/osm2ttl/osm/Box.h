// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_BOX_H_
#define OSM2TTL_OSM_BOX_H_

#include "osm2ttl/geometry/Box.h"

namespace osm2ttl {
namespace osm {

class Box {
 public:
  explicit Box(const osm2ttl::geometry::Box& box);
  [[nodiscard]] osm2ttl::geometry::Box geom() const;

 protected:
  osm2ttl::geometry::Box _geom;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_BOX_H_