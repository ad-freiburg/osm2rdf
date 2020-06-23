// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Ring.h"

#include <cmath>
#include <iostream>

#include "osmium/osm/box.hpp"
#include "osmium/osm/location.hpp"

// ____________________________________________________________________________
double osm2ttl::osm::Ring::area() const noexcept {
  const double AREA_FACTOR = 2.0;
  int64_t res = 0;
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    const osmium::Location& l1 = vertices[i];
    const osmium::Location& l2 = vertices[i + 1];
    res += (l1.x() * l2.y());
    res -= (l1.y() * l2.x());
  }
  return std::abs(res/AREA_FACTOR);
}

// ____________________________________________________________________________
osmium::Box osm2ttl::osm::Ring::bbox() const noexcept {
  osmium::Box box;
  for (const auto& v : vertices) {
    box.extend(v);
  }
  return box;
}

// ____________________________________________________________________________
osmium::Location osm2ttl::osm::Ring::centroid() const noexcept {
  const int AREA_FACTOR = 6;
  double c = AREA_FACTOR * area();

  int32_t x = 0;
  int32_t y = 0;
  for (size_t i = 0; i < vertices.size() - 1; ++i) {
    const osmium::Location& l1 = vertices[i];
    const osmium::Location& l2 = vertices[i + 1];
    const int32_t f = (l1.x() * l2.y() - l2.x() * l1.y());
    x += (l1.x() + l2.x()) * f;
    y += (l1.y() + l2.y()) * f;
  }
  return osmium::Location(x/c, y/c);
}

