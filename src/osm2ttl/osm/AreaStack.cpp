// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaStack.h"

#include <algorithm>

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/AreaHandler.h"

// ____________________________________________________________________________
osm2ttl::osm::AreaStack::AreaStack(osm2ttl::osm::AreaHandler* areaHandler) :
  _areaHandler(areaHandler) {
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaStack::add(const osm2ttl::osm::Area& area) {
  elements.push_back(area.id());
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaStack::add(uint64_t area) {
  elements.push_back(area);
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaStack::sort() {
  std::sort(elements.begin(), elements.end(),
    [this](uint64_t& i, uint64_t& j) -> bool {
    const osm2ttl::osm::Area a = _areaHandler->lookup(i);
    const osm2ttl::osm::Area b = _areaHandler->lookup(j);
    if (a.tagAdministrationLevel() >= 0 && b.tagAdministrationLevel() >= 0) {
      return a.tagAdministrationLevel() > b.tagAdministrationLevel();
    }
    // No other comparison found, move smaller to the left
    return a.vagueArea() < b.vagueArea();
  });
}
