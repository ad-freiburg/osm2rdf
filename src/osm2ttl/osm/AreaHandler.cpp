// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaHandler.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"

// ____________________________________________________________________________
osm2ttl::osm::AreaHandler::AreaHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer) : _config(config), _writer(writer) {
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::area(const osmium::Area& area) {
  if (_config.ignoreUnnamed && area.tags()["name"] != nullptr) {
    return;
  }
  osm2ttl::osm::Area a(area);
  _areas.emplace(a.id(), a);
  if (a.tagAdministrationLevel() != 0) {
    _stacks.push_back(a.id());
  }
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::sort() {
  std::vector<uint64_t>& stack = _stacks;
  std::sort(stack.begin(), stack.end(),
    [this](uint64_t& i, uint64_t& j) -> bool {
    const osm2ttl::osm::Area a = _areas.at(i);
    const osm2ttl::osm::Area b = _areas.at(j);
    if (a.tagAdministrationLevel() >= 0 && b.tagAdministrationLevel() >= 0) {
      return a.tagAdministrationLevel() > b.tagAdministrationLevel();
    }
    // No other comparison found, move smaller to the left
    return a.vagueArea() < b.vagueArea();
  });
}
