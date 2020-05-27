// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/osm/DumpHandler.h"

#include <ostream>

#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/nt/Writer.h"

// ____________________________________________________________________________
osm2nt::osm::DumpHandler::DumpHandler(std::ostream* os) {
  w = new osm2nt::nt::Writer{os};
}

// ____________________________________________________________________________
osm2nt::osm::DumpHandler::~DumpHandler() {
  delete w;
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::area(const osmium::Area& area) const {
  w->writeOsmArea(area);
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::node(const osmium::Node& node) const {
  w->writeOsmNode(node);
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::relation(const osmium::Relation& relation)
  const {
  w->writeOsmRelation(relation);
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::way(const osmium::Way& way) const {
  w->writeOsmWay(way);
}
