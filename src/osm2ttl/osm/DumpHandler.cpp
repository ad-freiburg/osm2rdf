// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/DumpHandler.h"

#include <ostream>
#include <string>

#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/ttl/Writer.h"

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/Way.h"

// ____________________________________________________________________________
osm2ttl::osm::DumpHandler::DumpHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer) : _config(config),
  _queue(_config.numThreadsRead, "DumpHandler"), _writer(writer) {
}

// ____________________________________________________________________________
osm2ttl::osm::DumpHandler::~DumpHandler() {
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::finish() {
  _queue.quit();
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::area(const osmium::Area& area) {
  if (_config.noAreaDump) {
    return;
  }
  if (area.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  osm2ttl::osm::Area a{area};
  _queue.dispatch([this, a]{
    _writer->writeArea(a);
  });
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::node(const osmium::Node& node) {
  if (_config.noNodeDump) {
    return;
  }
  if (node.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  osm2ttl::osm::Node n{node};
  _queue.dispatch([this, n]{
    _writer->writeNode(n);
  });
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::relation(const osmium::Relation& relation) {
  if (_config.noRelationDump) {
    return;
  }
  if (relation.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  osm2ttl::osm::Relation r{relation};
  _queue.dispatch([this, r]{
    _writer->writeRelation(r);
  });
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::way(const osmium::Way& way) {
  if (_config.noWayDump) {
    return;
  }
  if (way.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  osm2ttl::osm::Way w{way};
  _queue.dispatch([this, w]{
    _writer->writeWay(w);
  });
}
