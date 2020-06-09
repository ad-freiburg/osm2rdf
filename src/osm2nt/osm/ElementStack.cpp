// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/osm/ElementStack.h"

#include <algorithm>

#include "osm2nt/osm/Element.h"

// ____________________________________________________________________________
void osm2nt::osm::ElementStack::add(const osm2nt::osm::Element& element) {
  elements.push_back(element);
}

// ____________________________________________________________________________
void osm2nt::osm::ElementStack::sort() {
  std::sort(elements.begin(), elements.end(), _sort);
}

// ____________________________________________________________________________
bool osm2nt::osm::ElementStack::_sort(const osm2nt::osm::Element& i,
                                      const osm2nt::osm::Element& j) {
  // Both Elements have a valid administration level, move lower to the right
  if (i.tagAdministrationLevel >= 0 && j.tagAdministrationLevel >= 0) {
    return i.tagAdministrationLevel > j.tagAdministrationLevel;
  }
  // No other comparison found, move smaller to the left
  return i.vagueArea() < j.vagueArea();
}
