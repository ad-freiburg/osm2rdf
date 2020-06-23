// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaHandler.h"

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
  osm2ttl::osm::Area a(area);
  _areas.emplace(a.id(), a);
  for (const auto& p : stacksForArea(a)) {
  }
}

// ____________________________________________________________________________
std::vector<std::pair<size_t, size_t>>
osm2ttl::osm::AreaHandler::stacksForArea(const osm2ttl::osm::Area& area) {
  std::vector<std::pair<size_t, size_t>> result;

  return result;
}

// ____________________________________________________________________________
osm2ttl::osm::Area osm2ttl::osm::AreaHandler::lookup(uint64_t id) const {
  return _areas.at(id);
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::sort() {
}
