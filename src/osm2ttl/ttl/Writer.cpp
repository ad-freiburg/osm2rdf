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

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/WKTFactory.h"

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/Literal.h"

// ____________________________________________________________________________
osm2ttl::ttl::Writer::Writer(const osm2ttl::config::Config& config)
  : _config(config), _queue(_config.writerThreads) {
  _out = &std::cout;
  _factory = osm2ttl::osm::WKTFactory::create(_config);
}

// ____________________________________________________________________________
osm2ttl::ttl::Writer::~Writer() {
  close();
  delete _factory;
}

// ____________________________________________________________________________
bool osm2ttl::ttl::Writer::open() {
  if (!_config.output.empty()) {
    _outFile.open(_config.output);
    _out = &_outFile;
    return _outFile.is_open();
  }
  return true;
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::close() {
  if (_outFile.is_open()) {
    _outFile.close();
  }
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
void osm2ttl::ttl::Writer::writeHeader() {
  const std::lock_guard<std::mutex> lock(_outMutex);
  *_out << _config.outputFormat.header();
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
  _queue.dispatch([this, s, p, o]{
    const std::lock_guard<std::mutex> lock(_outMutex);
    *_out << _config.outputFormat.format(s);
    *_out << " ";
    *_out << _config.outputFormat.format(p);
    *_out << " ";
    *_out << _config.outputFormat.format(o);
    *_out << " .\n";
  });
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOsmiumArea(const osmium::Area& area) {
  osm2ttl::ttl::IRI s{area.from_way()?"osmway":"osmrel",
      std::to_string(area.orig_id())};

  // Do not add tags or type as this extends a relation or way
  writeTriple(s,
    osm2ttl::ttl::IRI("geo", "hasGeometry"),
    osm2ttl::ttl::Literal(_factory->create_multipolygon(area),
      osm2ttl::ttl::IRI("geo", "wktLiteral")));

  if (_config.metaData) {
  writeTriple(s,
    osm2ttl::ttl::IRI("osma", "from_way"),
    osm2ttl::ttl::Literal(area.from_way()?"true":"false"));

    writeTriple(s,
      osm2ttl::ttl::IRI("osma", "orig_id"),
      osm2ttl::ttl::Literal(std::to_string(area.orig_id()),
        osm2ttl::ttl::IRI("xsd", "integer")));

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
      osm2ttl::ttl::Literal(area.is_multipolygon()?"true":"false"));
  }

  if (_config.addBBox) {
    writeOsmiumBox(s, osm2ttl::ttl::IRI("osm", "bbox"), area.envelope());
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOSM2TTLArea(const osm2ttl::osm::Area& area) {
  osm2ttl::ttl::IRI s{area.fromWay()?"osmway":"osmrel",
      std::to_string(area.objId())};
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmiumBox(const S& s,
                                     const osm2ttl::ttl::IRI& p,
                                     const osmium::Box& box) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  writeTriple(s, p, osm2ttl::ttl::Literal(box));
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmiumLocation(const S& s,
                                            const osmium::Location& location) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  std::ostringstream loc;
  location.as_string_without_check(std::ostream_iterator<char>(loc));

  writeTriple(s,
    osm2ttl::ttl::IRI("geo", "hasGeometry"),
    osm2ttl::ttl::Literal(_factory->create_point(location),
      osm2ttl::ttl::IRI("geo", "wktLiteral")));
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOsmiumNode(const osmium::Node& node) {
  osm2ttl::ttl::IRI s{"osmnode", node};

  writeTriple(s,
    osm2ttl::ttl::IRI("rdf", "type"),
    osm2ttl::ttl::IRI("osm", "node"));

  writeOsmiumLocation(s, node.location());
  writeOsmiumTagList(s, node.tags());
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeOsmiumRelation(
  const osmium::Relation& relation) {
  osm2ttl::ttl::IRI s{"osmrel", relation};

  writeTriple(s,
    osm2ttl::ttl::IRI("rdf", "type"),
    osm2ttl::ttl::IRI("osm", "relation"));

  writeOsmiumTagList(s, relation.tags());
  writeOsmiumRelationMembers(s, relation.members());
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmiumRelationMembers(
    const S& s,
    const osmium::RelationMemberList& members) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  // If only basic data is requested, skip this.

  std::uint32_t i = 0;
  for (const osmium::RelationMember& member : members) {
    std::string role{member.role()};
    if (!role.empty() && role != "outer" && role != "inner") {
      writeTriple(s,
        osm2ttl::ttl::IRI("osmrel", role),
        osm2ttl::ttl::IRI("osm"
          + std::string((osmium::item_type_to_name(
            member.type()) == std::string("relation")) ? "rel" : "way"),
          member));
    }
    if (!_config.expandedData) {
      continue;
    }
    if (role.empty()) {
      role = "member";
    }
    osm2ttl::ttl::BlankNode b;
    writeTriple(s,
      osm2ttl::ttl::IRI("osmrel", "membership"),
      b);

    writeTriple(b,
      osm2ttl::ttl::IRI("osmrel", role),
      osm2ttl::ttl::IRI("osm"
        + std::string(osmium::item_type_to_name(member.type())),
        member));

    writeTriple(b,
      osm2ttl::ttl::IRI("osmm", "pos"),
      osm2ttl::ttl::Literal(std::to_string(++i),
                          osm2ttl::ttl::IRI("xsd", "integer")));
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmiumTag(const S& s,
                                     const osmium::Tag& tag) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  // No spaces allowed in tag keys (see 002.problem.nt)
  std::string key = std::string(tag.key());
  std::string tmp;
  tmp.reserve(key.size());
  for (size_t pos = 0; pos < key.size(); ++pos) {
    switch (key[pos]) {
      case ' ':
        tmp += "_";
        break;
      default:
        tmp += key[pos];
    }
  }
  if (_config.tagKeyType.find(tag.key()) != _config.tagKeyType.end()) {
    writeTriple(s,
      osm2ttl::ttl::IRI("osmt", tmp),
      osm2ttl::ttl::Literal(tag.value(),
        _config.tagKeyType.at(tag.key())));
  } else {
    writeTriple(s,
      osm2ttl::ttl::IRI("osmt", tmp),
      osm2ttl::ttl::Literal(tag.value()));
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmiumTagList(const S& s,
                                         const osmium::TagList& tags) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  for (const osmium::Tag& tag : tags) {
    writeOsmiumTag(s, tag);
    if (!_config.skipWikiLinks) {
      if (std::string(tag.key()) == "wikidata") {
        std::string value{tag.value()};
        // Only take first wikidata entry if ; is found
        auto end = value.find(';');
        if (end != std::string::npos) {
          value = value.erase(end);
        }
        // Remove all but Q and digits to ensuder Qdddddd format
        value.erase(remove_if(value.begin(), value.end(), [](char c) {
          return (!isdigit(c) && c != 'Q');
        }), value.end());

        writeTriple(s,
          osm2ttl::ttl::IRI("osm", tag.key()),
          osm2ttl::ttl::IRI("wd", value));
      }
      if (Writer::endsWith(tag.key(), "wikipedia") &&
          !Writer::contains(tag.key(), "fixme")) {
        std::string v = tag.value();
        auto pos = v.find(':');
        if (pos != std::string::npos) {
          std::string lang = v.substr(0, pos);
          std::string entry = v.substr(pos + 1);
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
void osm2ttl::ttl::Writer::writeOsmiumWay(const osmium::Way& way) {
  osm2ttl::ttl::IRI s{"osmway", way};

  writeTriple(s,
    osm2ttl::ttl::IRI("rdf", "type"),
    osm2ttl::ttl::IRI("osm", "way"));

  writeOsmiumTagList(s, way.tags());
  writeOsmiumWayNodeList(s, way.nodes());

  // Count unique points, this only checks direct duplicates.
  size_t numUniquePoints = 0;
  osmium::Location last;
  for (auto const *it = way.nodes().cbegin(); it != way.nodes().cend(); ++it) {
    if (last != it->location()) {
      last = it->location();
      numUniquePoints++;
    }
  }

  // Select geometry object in relation to the number of unique points.
  if (numUniquePoints > 3 && way.is_closed()) {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(_factory->create_polygon(way),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  } else if (numUniquePoints > 1) {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(_factory->create_linestring(way),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  } else {
    writeTriple(s,
      osm2ttl::ttl::IRI("geo", "hasGeometry"),
      osm2ttl::ttl::Literal(
        _factory->create_point(way.nodes()[0]),
        osm2ttl::ttl::IRI("geo", "wktLiteral")));
  }

  if (_config.metaData) {
    writeTriple(s,
      osm2ttl::ttl::IRI("osmway", "is_closed"),
      osm2ttl::ttl::Literal(way.is_closed()?"yes":"no"));
    writeTriple(s,
      osm2ttl::ttl::IRI("osmway", "nodeCount"),
      osm2ttl::ttl::Literal(std::to_string(way.nodes().size())));
    writeTriple(s,
      osm2ttl::ttl::IRI("osmway", "uniqueNodeCount"),
      osm2ttl::ttl::Literal(std::to_string(numUniquePoints)));
  }

  if (_config.addBBox) {
    writeOsmiumBox(s, osm2ttl::ttl::IRI("osm", "bbox"), way.envelope());
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeOsmiumWayNodeList(const S& s,
                                             const osmium::WayNodeList& nodes) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  // If only basic data is requested, skip this.
  if (!_config.expandedData) {
    return;
  }

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
