// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/Writer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "osmium/geom/factory.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/item_type.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/config/Config.h"

#include "osm2nt/nt/Triple.h"
#include "osm2nt/nt/Subject.h"
#include "osm2nt/nt/Predicate.h"
#include "osm2nt/nt/Object.h"
#include "osm2nt/nt/BlankNodeLabel.h"
#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/Literal.h"

#include "osm2nt/osm/SimplifyingWKTFactory.h"

// ____________________________________________________________________________
osm2nt::nt::Writer::Writer(const osm2nt::config::Config& config) {
  _config = config;
  _out = &std::cout;
  if (!_config.output.empty()) {
    _outFile.open(config.output);
    _out = &_outFile;
  }
}

// ____________________________________________________________________________
bool osm2nt::nt::Writer::tagKeyEndsWith(const osmium::Tag& tag,
                                        const std::string& needle) {
  std::string hay{tag.key()};
  // Everything shall match the empty string.
  if (needle.empty()) {
    return true;
  }
  // If searchstring is longer than the text, it can never match.
  if (hay.size() < needle.size()) {
    return false;
  }
  int offset = hay.size() - needle.size();
  for (int i = hay.size() - 1; i - offset >= 0; --i) {
    if (hay[i] != needle[i - offset]) {
      return false;
    }
  }
  return true;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeHeader() {
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeTriple(const osm2nt::nt::Triple& t) {
  *_out << t << '\n';
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmArea(const osmium::Area& area) {
  if (_config.ignoreUnnamed && area.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::Subject* s;
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;

  s = new osm2nt::nt::IRI("https://www.openstreetmap.org/area/", area);
  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "WKT");
  if (_config.simplifyWKT) {
    o = new osm2nt::nt::Literal(
      _simplifyingWktFactory.create_multipolygon(area));
  } else {
    o = new osm2nt::nt::Literal(_wktFactory.create_multipolygon(area));
  }
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "from_way");
  o = new osm2nt::nt::Literal(area.from_way()?"yes":"no");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "orig_id");
  o = new osm2nt::nt::Literal(std::to_string(area.orig_id()));
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "orig");
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
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;

  std::stringstream loc;
  location.as_string_without_check(std::ostream_iterator<char>(loc));

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/Location/", "direct");
  o = new osm2nt::nt::Literal(loc.str());
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/Location/", "WKT");
  if (_config.simplifyWKT) {
    o = new osm2nt::nt::Literal(_simplifyingWktFactory.create_point(location));
  } else {
    o = new osm2nt::nt::Literal(_wktFactory.create_point(location));
  }
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmNode(const osmium::Node& node) {
  if (_config.ignoreUnnamed && node.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::Subject* s;

  s = new osm2nt::nt::IRI("https://www.openstreetmap.org/node/", node);
  writeOsmLocation(s, node.location());
  writeOsmTagList(s, node.tags());
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmRelation(const osmium::Relation& relation) {
  if (_config.ignoreUnnamed && relation.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::Subject* s;

  s = new osm2nt::nt::IRI(
    "https://www.openstreetmap.org/relation/", relation);
  writeOsmTagList(s, relation.tags());
  writeOsmRelationMembers(s, relation.members());
  delete s;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmRelationMembers(
    const osm2nt::nt::Subject* s,
    const osmium::RelationMemberList& members) {
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/relation/", "member");
  for (const osmium::RelationMember& member : members) {
    o = new osm2nt::nt::IRI("https://www.openstreetmap.org/"
      + std::string(osmium::item_type_to_name(member.type()))  + "/", member);
    writeTriple(osm2nt::nt::Triple(s, p, o));
    delete o;
  }
  delete p;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmTag(const osm2nt::nt::Subject* s,
                                     const osmium::Tag& tag) {
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;
  // No spaces allowed in tag keys (see 002.problem.nt)
  std::string key = std::string(tag.key());
  std::stringstream tmp;
  for (size_t pos = 0; pos < key.size(); ++pos) {
    switch (key[pos]) {
      case ' ':
        tmp << "_";
        break;
      default:
        tmp << key[pos];
    }
  }
  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/wiki/key:", tmp.str());
  o = new osm2nt::nt::Literal(tag.value());
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmTagList(const osm2nt::nt::Subject* s,
                                         const osmium::TagList& tags) {
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;

  for (const osmium::Tag& tag : tags) {
    writeOsmTag(s, tag);
    if (_config.addWikiLinks) {
      if (Writer::tagKeyEndsWith(tag, "wikidata")) {
        p = new osm2nt::nt::IRI("https://www.openstreetmap.org/way/",
                                "wikidata");
        o = new osm2nt::nt::IRI("https://www.wikidata.org/wiki/", tag.value());
        writeTriple(osm2nt::nt::Triple(s, p, o));
        delete o;
        delete p;
      }
      if (Writer::tagKeyEndsWith(tag, "wikipedia")) {
        p = new osm2nt::nt::IRI("https://www.openstreetmap.org/way/",
                                "wikidata");
        o = new osm2nt::nt::IRI("https://www.wikipedia.org/wiki/", tag.value());
        writeTriple(osm2nt::nt::Triple(s, p, o));
        delete o;
        delete p;
      }
    }
  }
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmWay(const osmium::Way& way) {
  if (_config.ignoreUnnamed && way.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::Subject* s;
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;

  s = new osm2nt::nt::IRI("https://www.openstreetmap.org/way/", way);

  writeOsmTagList(s, way.tags());
  writeOsmWayNodeList(s, way.nodes());

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "is_closed");
  o = new osm2nt::nt::Literal(way.is_closed()?"yes":"no");
  writeTriple(osm2nt::nt::Triple(s, p, o));
  delete o;
  delete p;

  p = new osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT");
  if (way.nodes().size() > 3 && way.is_closed()) {
    if (_config.simplifyWKT) {
      o = new osm2nt::nt::Literal(_simplifyingWktFactory.create_polygon(way));
    } else {
      o = new osm2nt::nt::Literal(_wktFactory.create_polygon(way));
    }
  } else if (way.nodes().size() > 1) {
    if (_config.simplifyWKT) {
      o = new osm2nt::nt::Literal(
        _simplifyingWktFactory.create_linestring(
          way, osmium::geom::use_nodes::all));
    } else {
      o = new osm2nt::nt::Literal(
        _wktFactory.create_linestring(way, osmium::geom::use_nodes::all));
    }
  } else {
    if (_config.simplifyWKT) {
      o = new osm2nt::nt::Literal(
        _simplifyingWktFactory.create_point(way.nodes()[0]));
    } else {
      o = new osm2nt::nt::Literal(_wktFactory.create_point(way.nodes()[0]));
    }
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
  osm2nt::nt::Predicate* p;
  osm2nt::nt::Object* o;

  uint32_t i = 0;
  for (const osmium::NodeRef& nodeRef : nodes) {
    p = new osm2nt::nt::IRI(
      "https://www.openstreetmap.org/way/", "node");
    osm2nt::nt::BlankNodeLabel* b = new osm2nt::nt::BlankNodeLabel();
    writeTriple(osm2nt::nt::Triple(s, p, b));
    delete p;

    p = new osm2nt::nt::IRI(
      "https://www.openstreetmap.org/way/", "node");
    o = new osm2nt::nt::IRI(
      "https://www.openstreetmap.org/node/", nodeRef);
    writeTriple(osm2nt::nt::Triple(b, p, o));
    delete o;
    delete p;

    osm2nt::nt::IRI* t = new osm2nt::nt::IRI(
      "http://www.w3.org/2001/XMLSchema#", "integer");
    p = new osm2nt::nt::IRI(
      "https://www.openstreetmap.org/way/", "node/pos");
    o = new osm2nt::nt::Literal(std::to_string(++i), t);
    writeTriple(osm2nt::nt::Triple(b, p, o));
    delete t;
    delete o;
    delete p;
    delete b;
  }
}
