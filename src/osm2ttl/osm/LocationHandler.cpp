// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/LocationHandler.h"

#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/index/map/flex_mem.hpp"

#include "osm2ttl/config/Config.h"

// ____________________________________________________________________________
osm2ttl::osm::LocationHandler* osm2ttl::osm::LocationHandler::create(
  const osm2ttl::config::Config& config) {
  if (config.useRamForLocations) {
    return new osm2ttl::osm::LocationHandlerRAM(config);
  }
  return new osm2ttl::osm::LocationHandlerFS(config);
}

// ____________________________________________________________________________
void osm2ttl::osm::LocationHandler::firstPassDone() {
  _firstPassDone = true;
}

// ____________________________________________________________________________
template<typename T>
void osm2ttl::osm::LocationHandlerImpl<T>::node(const osmium::Node& node) {
  if (_firstPassDone) {
    return;
  }
  _handler.node(node);
}

// ____________________________________________________________________________
template<typename T>
void osm2ttl::osm::LocationHandlerImpl<T>::way(osmium::Way& way) {  // NOLINT
  _handler.way(way);
}

// ____________________________________________________________________________
template<typename T>
osm2ttl::osm::LocationHandlerImpl<T>::LocationHandlerImpl(
  const osm2ttl::config::Config& /*unused*/) : _handler(_index) {
  _handler.ignore_errors();
}

// ____________________________________________________________________________
osm2ttl::osm::LocationHandlerImpl<osmium::index::map::SparseFileArray<
  osmium::unsigned_object_id_type, osmium::Location>>::LocationHandlerImpl(
  const osm2ttl::config::Config& config) :
  _cacheFile(config.getTempPath("osmium", "n2l.cache")),
  _index(_cacheFile.fileDescriptor()), _handler(_index) {
  _handler.ignore_errors();
}

// ____________________________________________________________________________
void osm2ttl::osm::LocationHandlerImpl<osmium::index::map::SparseFileArray<
  osmium::unsigned_object_id_type, osmium::Location>>::node(
  const osmium::Node& node) {
  if (_firstPassDone) {
    return;
  }
  _handler.node(node);
}

// ____________________________________________________________________________
void osm2ttl::osm::LocationHandlerImpl<osmium::index::map::SparseFileArray<
  osmium::unsigned_object_id_type, osmium::Location>>::way(
  osmium::Way& way) {  // NOLINT
  _handler.way(way);
}
