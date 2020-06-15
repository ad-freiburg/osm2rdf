// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/osm/DumpHandler.h"

#include <iostream>
#include <ostream>
#include <string>

#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/config/Config.h"
#include "osm2nt/nt/Writer.h"

// ____________________________________________________________________________
osm2nt::osm::DumpHandler::DumpHandler(osm2nt::nt::Writer* writer) {
  _w = writer;
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::area(const osmium::Area& area) {
  _w->writeOsmArea(area);
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::node(const osmium::Node& node) {
  _w->writeOsmNode(node);
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::relation(const osmium::Relation& relation) {
  _w->writeOsmRelation(relation);
}

// ____________________________________________________________________________
void osm2nt::osm::DumpHandler::way(const osmium::Way& way) {
  _w->writeOsmWay(way);
}
