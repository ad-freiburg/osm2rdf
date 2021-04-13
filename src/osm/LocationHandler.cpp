// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/osm/LocationHandler.h"

#include <iostream>

#include "osm2ttl/config/Config.h"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/flex_mem.hpp"
#include "osmium/index/map/sparse_file_array.hpp"

// ____________________________________________________________________________
osm2ttl::osm::LocationHandler* osm2ttl::osm::LocationHandler::create(
    const osm2ttl::config::Config& config) {
  if (!config.storeLocationsOnDisk) {
    return new osm2ttl::osm::LocationHandlerRAM(config);
  }
  return new osm2ttl::osm::LocationHandlerFS(config);
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::osm::LocationHandlerImpl<T>::node(const osmium::Node& node) {
  _handler.node(node);
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::osm::LocationHandlerImpl<T>::way(osmium::Way& way) {  // NOLINT
  _handler.way(way);
}

// ____________________________________________________________________________
template <typename T>
osm2ttl::osm::LocationHandlerImpl<T>::LocationHandlerImpl(
    const osm2ttl::config::Config& /*unused*/)
    : _handler(_index) {
  _handler.ignore_errors();
}

// ____________________________________________________________________________
osm2ttl::osm::LocationHandlerImpl<osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::Location>>::
    LocationHandlerImpl(const osm2ttl::config::Config& config)
    : _cacheFile(config.getTempPath("osmium", "n2l.cache")),
      _index(_cacheFile.fileDescriptor()),
      _handler(_index) {
  _handler.ignore_errors();
}

// ____________________________________________________________________________
void osm2ttl::osm::LocationHandlerImpl<osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type,
    osmium::Location>>::node(const osmium::Node& node) {
  _handler.node(node);
}

// ____________________________________________________________________________
void osm2ttl::osm::LocationHandlerImpl<osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type,
    osmium::Location>>::way(osmium::Way& way) {  // NOLINT
  _handler.way(way);
}