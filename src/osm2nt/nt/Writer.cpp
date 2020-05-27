// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.
//
#include "osm2nt/nt/Writer.h"

#include <string>
#include <fstream>
#include <sstream>

#include "osmium/osm/area.hpp"
#include "osmium/osm/item_type.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"
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
  osm2nt::nt::Subject* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", area);

  osm2nt::nt::Predicate* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "WKT");
  osm2nt::nt::Object* o = new osm2nt::nt::Literal(
    wktFactory.create_multipolygon(area));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "from_way");
  o = new osm2nt::nt::Literal(area.from_way()?"yes":"no");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "orig_id");
  o = new osm2nt::nt::Literal(std::to_string(area.orig_id()));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "orig");
  o = new osm2nt::nt::IRI(
    std::string("https://www.openstreetmap.org/")
    +std::string(area.from_way()?"way":"relation")
    +std::string("/"),
    std::to_string(area.orig_id()));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "num_outer_rings");
  o = new osm2nt::nt::Literal(std::to_string(area.num_rings().first));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "num_inner_rings");
  o = new osm2nt::nt::Literal(std::to_string(area.num_rings().second));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/area/", "is_multipolygon");
  o = new osm2nt::nt::Literal(area.is_multipolygon()?"yes":"no");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "bbox");
  writeOsmBox(s, p, area.envelope());
  delete p;

  writeOsmTagList(s, area.tags());
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmBox(const osm2nt::nt::Subject* s,
                                     const osm2nt::nt::Predicate* p,
                                     const osmium::Box& box) {
  osm2nt::nt::Object* o = new osm2nt::nt::Literal(
    "POLYGON(("
    +std::to_string(box.bottom_left().lon_without_check())+" "
    +std::to_string(box.top_right().lat_without_check())+","
    +std::to_string(box.top_right().lon_without_check())+" "
    +std::to_string(box.top_right().lat_without_check())+","
    +std::to_string(box.top_right().lon_without_check())+" "
    +std::to_string(box.bottom_left().lat_without_check())+","
    +std::to_string(box.bottom_left().lon_without_check())+" "
    +std::to_string(box.bottom_left().lat_without_check())+"))");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmLocation(const osm2nt::nt::Subject* s,
                                          const osmium::Location& location) {
  std::stringstream loc;
  location.as_string_without_check(std::ostream_iterator<char>(loc));

  osm2nt::nt::Predicate* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/Location/", "direct");
  osm2nt::nt::Object* o = new osm2nt::nt::Literal(loc.str());
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/Location/", "WKT");
  o = new osm2nt::nt::Literal(
    wktFactory.create_point(location));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmNode(const osmium::Node& node) {
  osm2nt::nt::Subject* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/node/", node);
  writeOsmLocation(s, node.location());
  writeOsmTagList(s, node.tags());
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmRelation(const osmium::Relation& relation) {
  osm2nt::nt::Subject* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/relation/", relation);
  writeOsmTagList(s, relation.tags());
  writeOsmRelationMembers(s, relation.members());
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmRelationMembers(
    const osm2nt::nt::Subject* s,
    const osmium::RelationMemberList& members) {
  osm2nt::nt::Predicate* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/relation/", "member");
  osm2nt::nt::Object* o;
  for (const osmium::RelationMember& member : members) {
    o = new osm2nt::nt::IRI(
      "https://www.openstreetmap.org/"
      + std::string(osmium::item_type_to_name(member.type()))  + "/", member);
    writeTriple(osm2nt::nt::Triple(s, p, o));
    delete o;
  }
  delete p;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmTag(const osm2nt::nt::Subject* s,
                                     const osmium::Tag& tag) {
  osm2nt::nt::Predicate* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/wiki/key:", tag.key());
  osm2nt::nt::Object* o = new osm2nt::nt::Literal(tag.value());
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
  osm2nt::nt::Subject* s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/way/", way);

  writeOsmTagList(s, way.tags());
  writeOsmWayNodeList(s, way.nodes());

  osm2nt::nt::Predicate* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/way/", "is_closed");
  osm2nt::nt::Object* o = new osm2nt::nt::Literal(way.is_closed()?"yes":"no");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/way/", "WKT");
  if (way.nodes().size() > 3 && way.is_closed()) {
    o = new osm2nt::nt::Literal(wktFactory.create_polygon(way));
  } else if (way.nodes().size() > 1) {
    o = new osm2nt::nt::Literal(wktFactory.create_linestring(way));
  } else {
    o = new osm2nt::nt::Literal(wktFactory.create_point(way.nodes()[0]));
  }
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "bbox");
  writeOsmBox(s, p, way.envelope());
  delete p;

  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmWayNodeList(const osm2nt::nt::Subject* s,
                                             const osmium::WayNodeList& nodes) {
  osm2nt::nt::Predicate* p = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/way/", "node");
  osm2nt::nt::Object* o;
  for (const osmium::NodeRef& nodeRef : nodes) {
    o = new osm2nt::nt::IRI(
      "https://www.openstreetmap.org/node/", nodeRef);
    writeTriple(osm2nt::nt::Triple(s, p, o));
    delete o;
  }
  delete p;
}
