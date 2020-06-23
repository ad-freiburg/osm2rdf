// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_RING_H_
#define OSM2TTL_OSM_RING_H_

#include <vector>

#include "osmium/osm/box.hpp"
#include "osmium/osm/location.hpp"

namespace osm2ttl {
namespace osm {

struct Ring {
  double area() const noexcept;
  osmium::Box bbox() const noexcept;
  osmium::Location centroid() const noexcept;

  std::vector<osmium::Location> vertices;
};

struct OuterRing : public Ring {
  std::vector<Ring> inner;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_RING_H_
