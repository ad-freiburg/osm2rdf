// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.
//
#include "osm2nt/nt/Writer.h"

#include <string>
#include <fstream>
#include <sstream>

#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/nt/Triple.h"
#include "osm2nt/nt/Subject.h"
#include "osm2nt/nt/Predicate.h"
#include "osm2nt/nt/Object.h"
#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/Literal.h"

// ____________________________________________________________________________
osm2nt::nt::Writer::Writer(std::ostream* os) {
  out = os;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeTriple(const osm2nt::nt::Triple& t) {
  *out << t << '\n';
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmArea(const osmium::Area& area) {
  osm2nt::nt::IRI* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", area);
  osm2nt::nt::IRI* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "WKT");
  osm2nt::nt::Literal* o = new osm2nt::nt::Literal(
    wktFactory.create_multipolygon(area));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  writeOsmTagList(s, area.tags());
  delete o;
  delete p;
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmLocation(const osm2nt::nt::Subject* s,
                                          const osm2nt::nt::Predicate* p,
                                          const osmium::Location& location) {
  const osm2nt::nt::IRI* iri = static_cast<const osm2nt::nt::IRI*>(p);
  std::stringstream loc;
  location.as_string_without_check(std::ostream_iterator<char>(loc));
  std::stringstream wkt;
  wkt << "POINT(";
  location.as_string_without_check(std::ostream_iterator<char>(wkt), ' ');
  wkt << ")";
  osm2nt::nt::Literal* o = new osm2nt::nt::Literal(loc.str());
  osm2nt::nt::Literal* o2 = new osm2nt::nt::Literal(wkt.str());
  osm2nt::nt::Predicate* p2 = new osm2nt::nt::IRI(iri->prefix,
                                                  iri->value+"/WKT");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  writeTriple(osm2nt::nt::Triple(s, p2, o2));
  delete o2;
  delete o;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmNode(const osmium::Node& node) {
  osm2nt::nt::IRI* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/node/", node);
  osm2nt::nt::IRI* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/node/", "Location");
  writeOsmLocation(s, p, node.location());
  writeOsmTagList(s, node.tags());
  delete p;
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmRelation(const osmium::Relation& relation) {
  osm2nt::nt::IRI* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/relation/", relation);
  writeOsmTagList(s, relation.tags());
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmTag(const osm2nt::nt::Subject* s,
                                     const osmium::Tag& tag) {
  osm2nt::nt::IRI* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/wiki/key:", tag.key());
  osm2nt::nt::Literal* o = new osm2nt::nt::Literal(tag.value());
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmTagList(const osm2nt::nt::Subject* s,
                                         const osmium::TagList& tags) {
  for (const osmium::Tag& tag : tags) {
    writeOsmTag(s, tag);
  }
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmWay(const osmium::Way& way) {
  osm2nt::nt::IRI* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/way/", way);
  writeOsmTagList(s, way.tags());
  delete s;
}
