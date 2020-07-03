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

// ____________________________________________________________________________
osm2ttl::osm::DumpHandler::DumpHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer, osm2ttl::osm::AreaHandler* areaHandler,
  osm2ttl::osm::MembershipHandler* membershipHandler) :
  _config(config), _writer(writer), _areaHandler(areaHandler),
  _membershipHandler(membershipHandler) {
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::area(const osmium::Area& area) {
  if (_config.noAreaDump) {
    return;
  }
  if (area.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  _writer->writeOsmiumArea(area);
  _writer->writeOSM2TTLArea(osm2ttl::osm::Area(area));
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::node(const osmium::Node& node) {
  if (_config.noNodeDump) {
    return;
  }
  if (node.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  _writer->writeOsmiumNode(node);
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::relation(const osmium::Relation& relation) {
  if (_config.noRelationDump) {
    return;
  }
  if (!_config.addAreaSources &&
      _membershipHandler->isArea(relation)) {
    return;
  }
  if (relation.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  _writer->writeOsmiumRelation(relation);
}

// ____________________________________________________________________________
void osm2ttl::osm::DumpHandler::way(const osmium::Way& way) {
  if (_config.noWayDump) {
    return;
  }
  if (!_config.addAreaSources &&
      _membershipHandler->isArea(way)) {
    return;
  }
  if (way.tags().byte_size() == EMPTY_TAG_SIZE) {
    return;
  }
  _writer->writeOsmiumWay(way);
}
