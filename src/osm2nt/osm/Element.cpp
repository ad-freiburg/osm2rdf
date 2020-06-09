// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/osm/Element.h"

// ____________________________________________________________________________
double osm2nt::osm::Element::vagueArea() const {
  return (box.top_right().x() - box.bottom_left().x()) *
         (box.top_right().y() - box.bottom_left().y());
}

// ____________________________________________________________________________
bool osm2nt::osm::Element::vagueIntersects(const osm2nt::osm::Element& other)
  const {
  return box.contains(other.box.bottom_left())
    || box.contains(other.box.top_right());
}

// ____________________________________________________________________________
bool osm2nt::osm::Element::vagueContains(const osm2nt::osm::Element& other)
  const {
  return box.contains(other.box.bottom_left())
    && box.contains(other.box.top_right());
}

// ____________________________________________________________________________
bool osm2nt::osm::Element::intersects(const osm2nt::osm::Element& other) const {
  return false;
}

// ____________________________________________________________________________
bool osm2nt::osm::Element::contains(const osm2nt::osm::Element& other) const {
  return false;
}
