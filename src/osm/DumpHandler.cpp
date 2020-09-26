// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/DumpHandler.h"

#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/ttl/Writer.h"

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/Way.h"

// ____________________________________________________________________________
template<typename W>
osm2ttl::osm::DumpHandler<W>::DumpHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer<W>* writer) : _config(config),
  _writer(writer) {
}

// ____________________________________________________________________________
template<typename W>
template<typename T>
void osm2ttl::osm::DumpHandler<W>::write(const T& o) {
  _writer->write(o);
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::area(const osmium::Area& area) {
  if (_config.noDump) {
    return;
  }
  if (_config.noAreaDump) {
    return;
  }
  if (area.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  write(osm2ttl::osm::Area(area));
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::node(const osmium::Node& node) {
  if (_config.noDump) {
    return;
  }
  if (_config.noNodeDump) {
    return;
  }
  if (node.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  write(osm2ttl::osm::Node(node));
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::relation(const osmium::Relation& relation) {
  if (_config.noDump) {
    return;
  }
  if (_config.noRelationDump) {
    return;
  }
  if (relation.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  write(osm2ttl::osm::Relation(relation));
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::way(const osmium::Way& way) {
  if (_config.noDump) {
    return;
  }
  if (_config.noWayDump) {
    return;
  }
  if (way.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  write(osm2ttl::osm::Way(way));
}

// ____________________________________________________________________________
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::QLEVER>;
