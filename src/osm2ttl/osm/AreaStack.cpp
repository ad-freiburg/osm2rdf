// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaStack.h"

#include <algorithm>

#include "osm2ttl/osm/Area.h"

// ____________________________________________________________________________
void osm2ttl::osm::AreaStack::add(const osm2ttl::osm::Area& area) {
  elements.push_back(area);
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaStack::sort() {
  std::sort(elements.begin(), elements.end(), _sort);
}

// ____________________________________________________________________________
bool osm2ttl::osm::AreaStack::_sort(const osm2ttl::osm::Area& i,
                                    const osm2ttl::osm::Area& j) {
  // Both Elements have a valid administration level, move lower to the right
  if (i.tagAdministrationLevel() >= 0 && j.tagAdministrationLevel() >= 0) {
    return i.tagAdministrationLevel() > j.tagAdministrationLevel();
  }
  // No other comparison found, move smaller to the left
  return i.vagueArea() < j.vagueArea();
}
