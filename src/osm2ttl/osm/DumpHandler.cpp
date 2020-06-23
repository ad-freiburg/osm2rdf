// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/DumpHandler.h"

#include <iostream>
#include <ostream>
#include <string>

#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/ttl/Writer.h"

// ____________________________________________________________________________
osm2ttl::osm::DumpHandler::DumpHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer, osm2ttl::osm::AreaHandler* areaHandler) :
  _config(config), _writer(writer), _areaHandler(areaHandler) {
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::area(const osmium::Area& area) {
  if (_config.ignoreUnnamed && area.tags()["name"] == nullptr) {
    return;
  }
  _writer->writeOsmArea(area);
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::node(const osmium::Node& node) {
  if (_config.ignoreUnnamed && node.tags()["name"] == nullptr) {
    return;
  }
  _writer->writeOsmNode(node);
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::relation(const osmium::Relation& relation) {
  if (_config.ignoreUnnamed && relation.tags()["name"] == nullptr) {
    return;
  }
  _writer->writeOsmRelation(relation);
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::way(const osmium::Way& way) {
  if (_config.ignoreUnnamed && way.tags()["name"] == nullptr) {
    return;
  }
  _writer->writeOsmWay(way);
}
