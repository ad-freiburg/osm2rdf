// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/Writer.h"

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

#include "osm2ttl/config/Config.h"

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/Literal.h"

#include "osm2ttl/osm/SimplifyingWKTFactory.h"

// ____________________________________________________________________________
osm2ttl::ttl::Writer::Writer(const osm2ttl::config::Config& config) {
  _config = config;
  _out = &std::cout;
  if (!_config.output.empty()) {
    _outFile.open(config.output);
    _out = &_outFile;
  }
  // well-known prefixes
  _prefixes["geo"] = "http://www.opengis.net/ont/geosparql#";
  _prefixes["wd"] = "http://www.wikidata.org/entity/";
  _prefixes["xsd"] = "http://www.w3.org/2001/XMLSchema#";
  // osm prefixes
  // https://wiki.openstreetmap.org/wiki/Sophox#How_OSM_data_is_stored
  // https://github.com/Sophox/sophox/blob/master/osm2rdf/osmutils.py#L35-L39
  _prefixes["osmnode"] = "https://www.openstreetmap.org/node/";
  _prefixes["osmrel"] = "https://www.openstreetmap.org/relation/";
  _prefixes["osmt"] = "https://www.openstreetmap.org/wiki/Key:";
  _prefixes["osmway"] = "https://www.openstreetmap.org/way/";
  _prefixes["osmm"] = "https://www.openstreetmap.org/meta/";
  // own prefixes
  _prefixes["osm"] = "https://www.openstreetmap.org/";
  _prefixes["osma"] = "https://www.openstreetmap.org/area/";
  _prefixes["osml"] = "https://www.openstreetmap.org/location/";
}

// ____________________________________________________________________________
bool osm2ttl::ttl::Writer::contains(std::string_view s,
                                  std::string_view n) {
  if (n.empty()) {
    return true;
  }
  if (s.size() < n.size()) {
    return false;
  }
  return (s.find(n) != std::string::npos);
}

// ____________________________________________________________________________
bool osm2ttl::ttl::Writer::endsWith(std::string_view s,
                                  std::string_view n) {
  if (n.empty()) {
    return true;
  }
  if (s.size() < n.size()) {
    return false;
  }
  return (s.find(n, s.size() - n.size()) != std::string::npos);
}

// ____________________________________________________________________________
bool osm2ttl::ttl::Writer::startsWith(std::string_view s,
                                    std::string_view n) {
  if (n.empty()) {
    return true;
  }
  if (s.size() < n.size()) {
    return false;
  }
  return (s.rfind(n, 0) != std::string::npos);
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::Writer::urlencode(std::string_view s) {
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
std::string osm2ttl::ttl::Writer::urlescape(std::string_view s) {
  std::stringstream tmp;
  for (size_t pos = 0; pos < s.size(); ++pos) {
    // PN_LOCAL_ESC
    switch (s[pos]) {
      case '_':
        tmp << "\\_";
        break;
      case '~':
        tmp << "\\~";
        break;
      case '.':
        tmp << "\\.'";
        break;
      case '-':
        tmp << "\\-";
        break;
      case '!':
        tmp << "\\!";
        break;
      case '$':
        tmp << "\\$";
        break;
      case '&':
        tmp << "\\&";
        break;
      case '\'':
        tmp << "\\\'";
        break;
      case '(':
        tmp << "\\(";
        break;
      case ')':
        tmp << "\\)";
        break;
      case '*':
        tmp << "\\*";
        break;
      case '+':
        tmp << "\\+";
        break;
      case ',':
        tmp << "\\,";
        break;
      case ';':
        tmp << "\\;";
        break;
      case '=':
        tmp << "\\=";
        break;
      case '/':
        tmp << "\\/";
        break;
      case '?':
        tmp << "\\?";
        break;
      case '#':
        tmp << "\\#";
        break;
      case '@':
        tmp << "\\@";
        break;
      case '%':
        tmp << "\\%";
        break;
      default:
        tmp << s[pos];
    }
  }
  return tmp.str();
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeHeader() const {
  if (_config.outputFormat != osm2ttl::ttl::OutputFormat::TTL) {
    return;
  }
  for (const auto& p : _prefixes) {
      *_out << "@prefix " << p.first << ": <" << p.second << "> .\n";
  }
}

// ____________________________________________________________________________
template<typename S, typename O>
void osm2ttl::ttl::Writer::writeTriple(const S& s, const osm2ttl::ttl::IRI& p,
                                     const O& o) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  static_assert(std::is_same<O, osm2ttl::ttl::BlankNode>::value
                || std::is_same<O, osm2ttl::ttl::IRI>::value
                || std::is_same<O, osm2ttl::ttl::Literal>::value);
  write(s);
  *_out << " ";
  write(p);
  *_out << " ";
  write(o);
  *_out << " .\n";
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::ttl::BlankNode& b) {
  *_out << "_:" << b.getId();
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::ttl::IRI& i) {
  switch (_config.outputFormat) {
  case osm2ttl::ttl::OutputFormat::NT:
    // Lookup prefix, if not defined print as long IRI
    if (_prefixes.find(i.prefix()) != _prefixes.end()) {
      *_out << "<" << _prefixes[i.prefix()] << urlencode(i.value()) << ">";
      return;
    }
    *_out << "<" << i.prefix() << urlencode(i.value()) << ">";
    return;
  case osm2ttl::ttl::OutputFormat::TTL:
    // Lookup prefix, if not defined print as long IRI
    if (_prefixes.find(i.prefix()) != _prefixes.end()) {
      *_out << i.prefix() << ":" << urlescape(i.value());
      return;
    }
    *_out << "<" << i.prefix() << urlencode(i.value()) << ">";
    return;
  default:
    throw;
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::ttl::LangTag& l) {
  *_out << l.value();
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::ttl::Literal& l) {
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
void osm2ttl::ttl::Writer::writeOsmArea(const osmium::Area& area) {
  if (_config.ignoreUnnamed && area.tags()["name"] == nullptr) {
    return;
  }
  osm2ttl::ttl::IRI s{"osma", area};

  if (_config.simplifyWKT) {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(_simplifyingWktFactory.create_multipolygon(area),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  } else {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(_wktFactory.create_multipolygon(area),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  }

  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "from_way"),
    osm2ttl::ttl::Literal(area.from_way()?"yes":"no"));

  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "orig_id"),
    osm2ttl::ttl::Literal(std::to_string(area.orig_id())));

  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "orig"),
    osm2ttl::ttl::IRI(area.from_way()?"osmway":"osmrel",
      std::to_string(area.orig_id())));

  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "num_outer_rings"),
    osm2ttl::ttl::Literal(std::to_string(area.num_rings().first),
      osm2ttl::ttl::IRI("xsd", "integer")));

  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "num_inner_rings"),
    osm2ttl::ttl::Literal(std::to_string(area.num_rings().second),
      osm2ttl::ttl::IRI("xsd", "integer")));

  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "is_multipolygon"),
    osm2ttl::ttl::Literal(area.is_multipolygon()?"yes":"no"));

  writeOsmBox(s, osm2ttl::ttl::IRI("osma", "bbox"), area.envelope());

  writeOsmTagList(s, area.tags());
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmBox(const S& s,
                                     const osm2ttl::ttl::IRI& p,
                                     const osmium::Box& box) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  writeTriple(s, p, osm2ttl::ttl::Literal(box));
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmLocation(const S& s,
                                          const osmium::Location& location) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  std::stringstream loc;
  location.as_string_without_check(std::ostream_iterator<char>(loc));

  writeTriple(s,
    osm2ttl::ttl::IRI("osml", "direct"),
    osm2ttl::ttl::Literal(loc.str()));

  if (_config.simplifyWKT) {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(_simplifyingWktFactory.create_point(location),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  } else {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(_wktFactory.create_point(location),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOsmNode(const osmium::Node& node) {
  if (_config.ignoreUnnamed && node.tags()["name"] == nullptr) {
    return;
  }
  osm2ttl::ttl::IRI s{"osmnode", node};

  writeOsmLocation(s, node.location());
  writeOsmTagList(s, node.tags());
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOsmRelation(const osmium::Relation& relation) {
  if (_config.ignoreUnnamed && relation.tags()["name"] == nullptr) {
    return;
  }
  osm2ttl::ttl::IRI s{"osmrel", relation};

  writeOsmTagList(s, relation.tags());
  writeOsmRelationMembers(s, relation.members());
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmRelationMembers(
    const S& s,
    const osmium::RelationMemberList& members) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);

  std::uint32_t i = 0;
  for (const osmium::RelationMember& member : members) {
    osm2ttl::ttl::BlankNode b;
    writeTriple(s,
      osm2ttl::ttl::IRI("osmrel", "membership"),
      b);

    writeTriple(b,
      osm2ttl::ttl::IRI("osmrel", "member"),
      osm2ttl::ttl::IRI("osm"
        + std::string(osmium::item_type_to_name(member.type()))  + "/",
        member));

    writeTriple(b,
      osm2ttl::ttl::IRI("osmm", "pos"),
      osm2ttl::ttl::Literal(std::to_string(++i),
                          osm2ttl::ttl::IRI("xsd", "integer")));
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmTag(const S& s,
                                     const osmium::Tag& tag) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
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
    osm2ttl::ttl::IRI("osmt", tmp.str()),
    osm2ttl::ttl::Literal(tag.value()));
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmTagList(const S& s,
                                         const osmium::TagList& tags) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  for (const osmium::Tag& tag : tags) {
    writeOsmTag(s, tag);
    if (_config.addWikiLinks) {
      if (Writer::endsWith(tag.key(), "wikidata") &&
          !Writer::contains(tag.key(), "fixme")) {
        std::string value{tag.value()};
        size_t pos1 = 0;
        size_t pos2 = value.find(";");
        if (pos2 != std::string::npos) {
          while (pos2 != std::string::npos) {
            writeTriple(s,
              osm2ttl::ttl::IRI("osm", "wikidata"),
              osm2ttl::ttl::IRI("wd", value.substr(pos1, pos2-pos1)));
            pos1 = pos2 + 1;
            pos2 = value.find(";", pos1);
          }
          writeTriple(s,
            osm2ttl::ttl::IRI("osm", "wikidata"),
            osm2ttl::ttl::IRI("wd", value.substr(pos1)));
        } else {
          writeTriple(s,
            osm2ttl::ttl::IRI("osm", "wikidata"),
            osm2ttl::ttl::IRI("wd", value));
        }
      }
      if (Writer::endsWith(tag.key(), "wikipedia") &&
          !Writer::contains(tag.key(), "fixme")) {
        std::string v = tag.value();
        auto pos = v.find(':');
        if (pos != std::string::npos) {
          std::string lang = v.substr(0, pos);
          std::string entry = v.substr(pos);
          writeTriple(s,
            osm2ttl::ttl::IRI("osm", "wikipedia"),
            osm2ttl::ttl::IRI("https://"+lang+".wikipedia.org/wiki/", entry));
        } else {
          writeTriple(s,
            osm2ttl::ttl::IRI("osm", "wikipedia"),
            osm2ttl::ttl::IRI("https://www.wikipedia.org/wiki/", v));
        }
      }
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOsmWay(const osmium::Way& way) {
  if (_config.ignoreUnnamed && way.tags()["name"] == nullptr) {
    return;
  }
  osm2ttl::ttl::IRI s{"osmway", way};

  writeOsmTagList(s, way.tags());
  writeOsmWayNodeList(s, way.nodes());

  writeTriple(s,
    osm2ttl::ttl::IRI("osmway", "is_closed"),
    osm2ttl::ttl::Literal(way.is_closed()?"yes":"no"));

  if (way.nodes().size() > 3 && way.is_closed()) {
    if (_config.simplifyWKT) {
      writeTriple(s,
        osm2ttl::ttl::IRI("geo", "hasGeometry"),
        osm2ttl::ttl::Literal(_simplifyingWktFactory.create_polygon(way),
          osm2ttl::ttl::IRI("geo", "wktLiteral")));
    } else {
      writeTriple(s,
        osm2ttl::ttl::IRI("geo", "hasGeometry"),
        osm2ttl::ttl::Literal(_wktFactory.create_polygon(way),
          osm2ttl::ttl::IRI("geo", "wktLiteral")));
    }
  } else if (way.nodes().size() > 1) {
    if (_config.simplifyWKT) {
      writeTriple(s,
        osm2ttl::ttl::IRI("geo", "hasGeometry"),
        osm2ttl::ttl::Literal(_simplifyingWktFactory.create_linestring(
          way, osmium::geom::use_nodes::all),
          osm2ttl::ttl::IRI("geo", "wktLiteral")));
    } else {
      writeTriple(s,
        osm2ttl::ttl::IRI("geo", "hasGeometry"),
        osm2ttl::ttl::Literal(_wktFactory.create_linestring(
          way, osmium::geom::use_nodes::all),
          osm2ttl::ttl::IRI("geo", "wktLiteral")));
    }
  } else {
    if (_config.simplifyWKT) {
      writeTriple(s,
        osm2ttl::ttl::IRI("geo", "hasGeometry"),
        osm2ttl::ttl::Literal(
          _simplifyingWktFactory.create_point(way.nodes()[0]),
          osm2ttl::ttl::IRI("geo", "wktLiteral")));
    } else {
      writeTriple(s,
        osm2ttl::ttl::IRI("geo", "hasGeometry"),
        osm2ttl::ttl::Literal(_wktFactory.create_point(way.nodes()[0]),
          osm2ttl::ttl::IRI("geo", "wktLiteral")));
    }
  }

  writeOsmBox(s, osm2ttl::ttl::IRI("osmway", "bbox"), way.envelope());
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmWayNodeList(const S& s,
                                             const osmium::WayNodeList& nodes) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  uint32_t i = 0;
  for (const osmium::NodeRef& nodeRef : nodes) {
    osm2ttl::ttl::BlankNode b;
    writeTriple(s, osm2ttl::ttl::IRI("osmway", "node"), b);

    writeTriple(b,
      osm2ttl::ttl::IRI("osmway", "node"),
      osm2ttl::ttl::IRI("osmnode", nodeRef));

    writeTriple(b,
      osm2ttl::ttl::IRI("osmm", "pos"),
      osm2ttl::ttl::Literal(std::to_string(++i),
        osm2ttl::ttl::IRI("xsd", "integer")));
  }
}
