// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Element.h"

// ____________________________________________________________________________
double osm2ttl::osm::Element::vagueArea() const {
  return (box.top_right().x() - box.bottom_left().x()) *
         (box.top_right().y() - box.bottom_left().y());
}

// ____________________________________________________________________________
bool osm2ttl::osm::Element::vagueIntersects(const osm2ttl::osm::Element& other)
  const {
  return box.contains(other.box.bottom_left())
    || box.contains(other.box.top_right());
}

// ____________________________________________________________________________
bool osm2ttl::osm::Element::vagueContains(const osm2ttl::osm::Element& other)
  const {
  return box.contains(other.box.bottom_left())
    && box.contains(other.box.top_right());
}

// ____________________________________________________________________________
bool osm2ttl::osm::Element::intersects(const osm2ttl::osm::Element& other)
  const {
  return false;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Element::contains(const osm2ttl::osm::Element& other) const {
  return false;
}
