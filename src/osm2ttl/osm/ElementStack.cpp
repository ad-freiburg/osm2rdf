// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/ElementStack.h"

#include <algorithm>

#include "osm2ttl/osm/Element.h"

// ____________________________________________________________________________
void osm2ttl::osm::ElementStack::add(const osm2ttl::osm::Element& element) {
  elements.push_back(element);
}

// ____________________________________________________________________________
void osm2ttl::osm::ElementStack::sort() {
  std::sort(elements.begin(), elements.end(), _sort);
}

// ____________________________________________________________________________
bool osm2ttl::osm::ElementStack::_sort(const osm2ttl::osm::Element& i,
                                       const osm2ttl::osm::Element& j) {
  // Both Elements have a valid administration level, move lower to the right
  if (i.tagAdministrationLevel >= 0 && j.tagAdministrationLevel >= 0) {
    return i.tagAdministrationLevel > j.tagAdministrationLevel;
  }
  // No other comparison found, move smaller to the left
  return i.vagueArea() < j.vagueArea();
}
