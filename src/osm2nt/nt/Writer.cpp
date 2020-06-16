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

#include "osm2nt/nt/BlankNode.h"
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
std::string osm2nt::nt::Writer::urlencode(const std::string& s) {
  std::stringstream tmp;
  for (size_t pos = 0; pos < s.size(); ++pos) {
    switch (s[pos]) {
      case ' ':
        tmp << "%20";
        break;
      case '\"':
        tmp << "%22";
        break;
      case '\'':
        tmp << "%24";
        break;
      case '%':
        tmp << "%25";
        break;
      case '&':
        tmp << "%26";
        break;
      case '(':
        tmp << "%28";
        break;
      case ')':
        tmp << "%29";
        break;
      case ',':
        tmp << "%2C";
        break;
      case '<':
        tmp << "%3C";
        break;
      case '>':
        tmp << "%3E";
        break;
      case '|':
        tmp << "%7C";
        break;
      default:
        tmp << s[pos];
    }
  }
  return tmp.str();
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeHeader() {
}

// ____________________________________________________________________________
template<typename S, typename P, typename O>
void osm2nt::nt::Writer::writeTriple(const S& s, const P& p, const O& o) {
  write(s);
  *_out << " ";
  write(p);
  *_out << " ";
  write(o);
  *_out << " .\n";
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::write(const osm2nt::nt::BlankNode& b) {
  *_out << "_:" << b.getId();
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::write(const osm2nt::nt::IRI& i) {
  switch (_config.outputFormat) {
  case osm2nt::nt::OutputFormat::NT:
    *_out << "<" << i.prefix() << urlencode(i.value()) << ">";
    break;
  case osm2nt::nt::OutputFormat::TTL:
    *_out << "<" << i.prefix() << urlencode(i.value()) << ">";
    break;
  default:
    throw;
  }
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::write(const osm2nt::nt::LangTag& l) {
  *_out << l.value();
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::write(const osm2nt::nt::Literal& l) {
  *_out << "\"";
  // Escape value
  std::string value = l.value();
  for (size_t pos = 0; pos < value.size(); ++pos) {
    switch (value[pos]) {
      case '\\':
        *_out << "\\\\";
        break;
      case '\n':
        *_out << "\\n";
        break;
      case '"':
        *_out << "\\\"";
        break;
      case '\r':
        *_out << "\\r";
        break;
      default:
        *_out << value[pos];
    }
  }
  *_out << "\"";
  if (auto iri = l.iri()) {
    *_out << "^^";
    write(*iri);
  }
  if (auto langTag = l.langTag()) {
    *_out << "@";
    write(*langTag);
  }
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmArea(const osmium::Area& area) {
  if (_config.ignoreUnnamed && area.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::IRI s{"https://www.openstreetmap.org/area/", area};

  if (_config.simplifyWKT) {
    writeTriple(s,
      osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "WKT"),
      osm2nt::nt::Literal(_simplifyingWktFactory.create_multipolygon(area)));
  } else {
    writeTriple(s,
      osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "WKT"),
      osm2nt::nt::Literal(_wktFactory.create_multipolygon(area)));
  }


  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "from_way"),
    osm2nt::nt::Literal(area.from_way()?"yes":"no"));

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "orig_id"),
    osm2nt::nt::Literal(std::to_string(area.orig_id())));

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "orig"),
    osm2nt::nt::IRI(
      std::string("https://www.openstreetmap.org/")
      +std::string(area.from_way()?"way":"relation")
      +std::string("/"),
      std::to_string(area.orig_id())));

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "num_outer_rings"),
    osm2nt::nt::Literal(std::to_string(area.num_rings().first)));

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "num_inner_rings"),
    osm2nt::nt::Literal(std::to_string(area.num_rings().second)));

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "is_multipolygon"),
    osm2nt::nt::Literal(area.is_multipolygon()?"yes":"no"));

  writeOsmBox(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/area/", "bbox"),
    area.envelope());

  writeOsmTagList(s, area.tags());
}

// ____________________________________________________________________________
template<typename S>
void osm2nt::nt::Writer::writeOsmBox(const S& s,
                                     const osm2nt::nt::IRI& p,
                                     const osmium::Box& box) {
  writeTriple(s, p, osm2nt::nt::Literal(box));
}

// ____________________________________________________________________________
template<typename S>
void osm2nt::nt::Writer::writeOsmLocation(const S& s,
                                          const osmium::Location& location) {
  std::stringstream loc;
  location.as_string_without_check(std::ostream_iterator<char>(loc));

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/Location/", "direct"),
    osm2nt::nt::Literal(loc.str()));

  if (_config.simplifyWKT) {
    writeTriple(s,
      osm2nt::nt::IRI("https://www.openstreetmap.org/Location/", "WKT"),
      osm2nt::nt::Literal(_simplifyingWktFactory.create_point(location)));
  } else {
    writeTriple(s,
      osm2nt::nt::IRI("https://www.openstreetmap.org/Location/", "WKT"),
      osm2nt::nt::Literal(_wktFactory.create_point(location)));
  }
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmNode(const osmium::Node& node) {
  if (_config.ignoreUnnamed && node.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::IRI s{"https://www.openstreetmap.org/node/", node};

  writeOsmLocation(s, node.location());
  writeOsmTagList(s, node.tags());
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmRelation(const osmium::Relation& relation) {
  if (_config.ignoreUnnamed && relation.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::IRI s{"https://www.openstreetmap.org/relation/", relation};

  writeOsmTagList(s, relation.tags());
  writeOsmRelationMembers(s, relation.members());
}

// ____________________________________________________________________________
template<typename S>
void osm2nt::nt::Writer::writeOsmRelationMembers(
    const S& s,
    const osmium::RelationMemberList& members) {

  std::uint32_t i = 0;
  for (const osmium::RelationMember& member : members) {
    osm2nt::nt::BlankNode b;
    writeTriple(s,
      osm2nt::nt::IRI("https://www.openstreetmap.org/relation/", "membership"),
      b);

    writeTriple(b,
      osm2nt::nt::IRI("https://www.openstreetmap.org/relation/", "member"),
      osm2nt::nt::IRI("https://www.openstreetmap.org/"
        + std::string(osmium::item_type_to_name(member.type()))  + "/",
        member));

    writeTriple(b,
      osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "pos"),
      osm2nt::nt::Literal(std::to_string(++i),
                          osm2nt::nt::IRI("http://www.w3.org/2001/XMLSchema#",
                                          "integer")));
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2nt::nt::Writer::writeOsmTag(const S& s,
                                     const osmium::Tag& tag) {
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
  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/wiki/key:", tmp.str()),
    osm2nt::nt::Literal(tag.value()));
}

// ____________________________________________________________________________
template<typename S>
void osm2nt::nt::Writer::writeOsmTagList(const S& s,
                                         const osmium::TagList& tags) {
  for (const osmium::Tag& tag : tags) {
    writeOsmTag(s, tag);
    if (_config.addWikiLinks) {
      if (Writer::tagKeyEndsWith(tag, "wikidata")) {
        writeTriple(s,
          osm2nt::nt::IRI("https://www.openstreetmap.org/", "wikidata"),
          osm2nt::nt::IRI("https://www.wikidata.org/wiki/", tag.value()));
      }
      if (Writer::tagKeyEndsWith(tag, "wikipedia")) {
        std::string v = tag.value();
        auto pos = v.find(':');
        if (pos != std::string::npos) {
          std::string lang = v.substr(0, pos);
          std::string entry = v.substr(pos);
          writeTriple(s,
            osm2nt::nt::IRI("https://www.openstreetmap.org/", "wikipedia"),
            osm2nt::nt::IRI("https://"+lang+".wikipedia.org/wiki/", entry));
        } else {
          writeTriple(s,
            osm2nt::nt::IRI("https://www.openstreetmap.org/", "wikipedia"),
            osm2nt::nt::IRI("https://www.wikipedia.org/wiki/", v));
        }
      }
    }
  }
}

// ____________________________________________________________________________
void osm2nt::nt::Writer::writeOsmWay(const osmium::Way& way) {
  if (_config.ignoreUnnamed && way.tags()["name"] == nullptr) {
    return;
  }
  osm2nt::nt::IRI s{"https://www.openstreetmap.org/way/", way};

  writeOsmTagList(s, way.tags());
  writeOsmWayNodeList(s, way.nodes());

  writeTriple(s,
    osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "is_closed"),
    osm2nt::nt::Literal(way.is_closed()?"yes":"no"));

  if (way.nodes().size() > 3 && way.is_closed()) {
    if (_config.simplifyWKT) {
      writeTriple(s,
        osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT"),
        osm2nt::nt::Literal(_simplifyingWktFactory.create_polygon(way)));
    } else {
      writeTriple(s,
        osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT"),
        osm2nt::nt::Literal(_wktFactory.create_polygon(way)));
    }
  } else if (way.nodes().size() > 1) {
    if (_config.simplifyWKT) {
      writeTriple(s,
        osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT"),
        osm2nt::nt::Literal(_simplifyingWktFactory.create_linestring(
          way, osmium::geom::use_nodes::all)));
    } else {
      writeTriple(s,
        osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT"),
        osm2nt::nt::Literal(_wktFactory.create_linestring(
          way, osmium::geom::use_nodes::all)));
    }
  } else {
    if (_config.simplifyWKT) {
      writeTriple(s,
        osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT"),
        osm2nt::nt::Literal(
          _simplifyingWktFactory.create_point(way.nodes()[0])));
    } else {
      writeTriple(s,
        osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "WKT"),
        osm2nt::nt::Literal(_wktFactory.create_point(way.nodes()[0])));
    }
  }

  writeOsmBox(s,
              osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "bbox"),
              way.envelope());
}

// ____________________________________________________________________________
template<typename S>
void osm2nt::nt::Writer::writeOsmWayNodeList(const S& s,
                                             const osmium::WayNodeList& nodes) {
  uint32_t i = 0;
  for (const osmium::NodeRef& nodeRef : nodes) {
    osm2nt::nt::BlankNode b;
    writeTriple(s,
      osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "node"),
      b);

    writeTriple(b,
      osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "node"),
      osm2nt::nt::IRI("https://www.openstreetmap.org/node/", nodeRef));

    writeTriple(b,
      osm2nt::nt::IRI("https://www.openstreetmap.org/way/", "node/pos"),
      osm2nt::nt::Literal(std::to_string(++i),
        osm2nt::nt::IRI("http://www.w3.org/2001/XMLSchema#", "integer")));
  }
}
