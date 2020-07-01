// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaHandler.h"

#include <filesystem>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/util/progress_bar.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/CacheFile.h"
#include "osm2ttl/ttl/Writer.h"

// ____________________________________________________________________________
osm2ttl::osm::AreaHandler::AreaHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer) : _config(config),
  _writer(writer), _areasFile(config.getTempPath("osm2ttl", "areas.cache")) {
  _areas =
  osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osm2ttl::osm::Area>(
      _areasFile.fileDescriptor());
}

// ____________________________________________________________________________
osm2ttl::osm::AreaHandler::~AreaHandler() {
  _areasFile.close();
  _areasFile.remove();
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::area(const osmium::Area& area) {
  // Only keep administrative areas... turn this into groups of attributes
  if (area.tags()["boundary"] == nullptr
      || std::string(area.tags()["boundary"]) != "administrative") {
    return;
  }
  osm2ttl::osm::Area a(area);
  _areas.set(a.id(), a);
  /*
  if (a.tagAdministrationLevel() != 0) {
    _stacks.push_back(a.id());
  }
  */
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::sort() {
  std::cerr << " sorting " << _areas.size() << " areas ... ";
  _areas.sort();
  std::cerr << "done\n" << " sorting " << "1" << " grid-area-stack(s)"
    << std::endl;

  osmium::ProgressBar progressBar{1, true};
  progressBar.update(0);
  std::vector<uint64_t>& stack = _stacks;
  std::sort(stack.begin(), stack.end(),
    [this](uint64_t& i, uint64_t& j) -> bool {
    const osm2ttl::osm::Area a = _areas.get(i);
    const osm2ttl::osm::Area b = _areas.get(j);
    return a < b;
  });
  progressBar.done();
}
