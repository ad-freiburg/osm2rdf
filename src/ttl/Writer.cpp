// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/Writer.h"

#include <osm2ttl/ttl/Constants.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "boost/geometry.hpp"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Tag.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/item_type.hpp"

static const int k0xB7 = 0xB7;
// ____________________________________________________________________________
template <typename T>
osm2ttl::ttl::Writer<T>::Writer(const osm2ttl::config::Config& config)
    : _config(config) {
  _out = &std::cout;

  // Generate constants
  osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__GEOSPARQL, "hasGeometry");
  osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__GEOSPARQL, "wktLiteral");
  osm2ttl::ttl::constants::IRI__OGC_CONTAINS =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "contains");
  osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "contained_by");
  osm2ttl::ttl::constants::IRI__OSM_META__POS =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_META, "pos");
  osm2ttl::ttl::constants::IRI__OSMWAY_ISCLOSED =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, "is_closed");
  osm2ttl::ttl::constants::IRI__OSMWAY_NODE =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, "node");
  osm2ttl::ttl::constants::IRI__OSMWAY_NODECOUNT =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, "nodeCount");
  osm2ttl::ttl::constants::IRI__OSMWAY_UNIQUENODECOUNT = generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, "uniqueNodeCount");
  osm2ttl::ttl::constants::IRI__OSM_ENVELOPE =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "envelope");
  osm2ttl::ttl::constants::IRI__OSM_NODE =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "node");
  osm2ttl::ttl::constants::IRI__OSM_RELATION =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "relation");
  osm2ttl::ttl::constants::IRI__OSM_WAY =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "way");
  osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "wikipedia");
  osm2ttl::ttl::constants::IRI__RDF_TYPE =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, "type");
  osm2ttl::ttl::constants::IRI__XSD_INTEGER =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__XML_SCHEMA, "integer");

  osm2ttl::ttl::constants::LITERAL__NO = generateLiteral("no", "");
  osm2ttl::ttl::constants::LITERAL__YES = generateLiteral("yes", "");
}

// ____________________________________________________________________________
template <typename T>
osm2ttl::ttl::Writer<T>::~Writer() {
  close();
}

// ____________________________________________________________________________
template <typename T>
bool osm2ttl::ttl::Writer<T>::open() {
  if (!_config.output.empty()) {
    _outFile.open(_config.output);
    _out = &_outFile;
    return _outFile.is_open();
  }
  return true;
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::close() {
  if (_outFile.is_open()) {
    _outFile.close();
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeHeader() {
  const std::lock_guard<std::mutex> lock(_outMutex);
  for (const auto& [prefix, iriref] : _prefixes) {
    *_out << "@prefix " << prefix << ": <" << iriref << "> .\n";
  }
}

// ____________________________________________________________________________
template <>
void osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::writeHeader() {}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::generateBlankNode() {
  return "_:" + std::to_string(_blankNodeCounter++);
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::generateIRI(std::string_view p,
                                                 uint64_t v) {
  return generateIRI(p, std::to_string(v));
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::generateIRI(std::string_view p,
                                                 std::string_view v) {
  auto begin = std::find_if(v.begin(), v.end(),
                            [](int c) { return std::isspace(c) == 0; });
  auto end = std::find_if(v.rbegin(), v.rend(),
                          [](int c) { return std::isspace(c) == 0; });
  // Trim strings
  return formatIRI(
      p, v.substr(begin - v.begin(), std::distance(begin, end.base())));
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::generateLangTag(std::string_view s) {
  bool allowDigits = false;
  bool ok = true;
  for (size_t pos = 0; pos < s.length(); pos++) {
    if (pos == 0 && s[0] == '-') {
      ok = false;
      break;
    }
    if ((s[pos] >= 'a' && s[pos] <= 'z') || (s[pos] >= 'A' && s[pos] <= 'Z') ||
        (s[pos] >= '0' && s[pos] <= '9' && allowDigits)) {
    } else if (s[pos] == '-') {
      allowDigits = true;
    } else {
      ok = false;
      break;
    }
  }
  std::string tmp = "@";
  tmp.reserve(s.size() + 1);
  if (ok) {
    tmp += s;
  }
  return tmp;
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::generateLiteral(std::string_view v,
                                                     std::string_view s) {
  return STRING_LITERAL_QUOTE(v) + std::string(s);
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeTriple(const std::string& s,
                                          const std::string& p,
                                          const std::string& o) {
#pragma omp critical
  { *_out << s + " " + p + " " + o + " .\n"; }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::write(const osm2ttl::osm::Area& area) {
  std::string s = generateIRI(
      area.fromWay() ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                     : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
      area.objId());

  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     area.geom());

  if (_config.addEnvelope) {
    writeBox(s, osm2ttl::ttl::constants::IRI__OSM_ENVELOPE, area.envelope());
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::write(const osm2ttl::osm::Node& node) {
  std::string s =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, node.id());

  writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
              osm2ttl::ttl::constants::IRI__OSM_NODE);

  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     node.geom());

  writeTagList(s, node.tags());
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::write(const osmium::Node& node) {
  std::string s = generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,
                              node.positive_id());

  writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
              osm2ttl::ttl::constants::IRI__OSM_NODE);

  auto loc = node.location();
  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     osm2ttl::geometry::Location{loc.lon(), loc.lat()});

  writeTagList(s, node.tags());
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::write(const osm2ttl::osm::Relation& relation) {
  std::string s = generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                              relation.id());

  writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
              osm2ttl::ttl::constants::IRI__OSM_RELATION);

  writeTagList(s, relation.tags());

  for (const auto& member : relation.members()) {
    const std::string& role = member.role();
    if (role != "outer" && role != "inner") {
      std::string type = osm2ttl::ttl::constants::NAMESPACE__OSM;
      if (member.type() == osm2ttl::osm::RelationMemberType::NODE) {
        type = osm2ttl::ttl::constants::NAMESPACE__OSM_NODE;
      } else if (member.type() == osm2ttl::osm::RelationMemberType::RELATION) {
        type = osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION;
      } else if (member.type() == osm2ttl::osm::RelationMemberType::WAY) {
        type = osm2ttl::ttl::constants::NAMESPACE__OSM_WAY;
      }
      writeTriple(
          s,
          generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, role),
          generateIRI(type, member.id()));
    }
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::write(const osmium::Relation& relation) {
  std::string s = generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                              relation.positive_id());

  writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
              osm2ttl::ttl::constants::IRI__OSM_RELATION);

  writeTagList(s, relation.tags());

  for (const auto& member : relation.members()) {
    const std::string& role = member.role();
    if (role != "outer" && role != "inner") {
      std::string type = osm2ttl::ttl::constants::NAMESPACE__OSM;
      switch (member.type()) {
        case osmium::item_type::node:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM_NODE;
          break;
        case osmium::item_type::relation:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION;
          break;
        case osmium::item_type::way:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM_WAY;
          break;
        default:
          break;
      }
      writeTriple(s,
                  generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                              role.empty() ? "member" : role),
                  generateIRI(type, member.positive_ref()));
    }
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::write(const osm2ttl::osm::Way& way) {
  std::string s =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, way.id());

  writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
              osm2ttl::ttl::constants::IRI__OSM_WAY);

  writeTagList(s, way.tags());

  if (_config.expandedData) {
    size_t i = 0;
    for (const auto& node : way.nodes()) {
      std::string blankNode = generateBlankNode();
      writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_NODE, blankNode);

      writeTriple(
          blankNode, osm2ttl::ttl::constants::IRI__OSMWAY_NODE,
          generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, node.id()));

      writeTriple(
          blankNode, osm2ttl::ttl::constants::IRI__OSM_META__POS,
          generateLiteral(std::to_string(++i),
                          "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
    }
  }

  osm2ttl::geometry::Linestring locations{way.geom()};
  size_t numUniquePoints = locations.size();
  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     locations);

  if (_config.metaData) {
    writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_ISCLOSED,
                way.closed() ? osm2ttl::ttl::constants::LITERAL__YES
                             : osm2ttl::ttl::constants::LITERAL__NO);
    writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_NODECOUNT,
                generateLiteral(std::to_string(way.nodes().size()), ""));
    writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_UNIQUENODECOUNT,
                generateLiteral(std::to_string(numUniquePoints), ""));
  }

  if (_config.addEnvelope) {
    writeBox(s, osm2ttl::ttl::constants::IRI__OSM_ENVELOPE, way.envelope());
  }
}

// ____________________________________________________________________________
template <typename T>
template <typename G>
void osm2ttl::ttl::Writer<T>::writeBoostGeometry(const std::string& s,
                                                 const std::string& p,
                                                 const G& g) {
  const double onePercent = 0.01;
  G geom{g};
  if (_config.wktSimplify > 0 &&
      boost::geometry::num_points(g) > _config.wktSimplify) {
    osm2ttl::geometry::Box box;
    boost::geometry::envelope(geom, box);
    boost::geometry::simplify(
        g, geom,
        std::min(
            boost::geometry::get<boost::geometry::max_corner, 0>(box) -
                boost::geometry::get<boost::geometry::min_corner, 0>(box),
            boost::geometry::get<boost::geometry::max_corner, 1>(box) -
                boost::geometry::get<boost::geometry::min_corner, 1>(box)) /
            (onePercent * _config.wktDeviation));
    // If empty geometry -> use original
    if (!boost::geometry::is_valid(geom) || boost::geometry::is_empty(geom)) {
      geom = g;
    }
  }
  std::ostringstream tmp;
  tmp << std::setprecision(_config.wktPrecision) << boost::geometry::wkt(geom);
  writeTriple(s, p,
              "\"" + tmp.str() + "\"^^" +
                  osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeBox(const std::string& s,
                                       const std::string& p,
                                       const osm2ttl::osm::Box& box) {
  writeBox(s, p, box.geom());
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeBox(const std::string& s,
                                       const std::string& p,
                                       const osm2ttl::geometry::Box& box) {
  // Box can not be simplified -> output directly.
  std::ostringstream tmp;
  tmp << boost::geometry::wkt(box);
  writeTriple(s, p,
              "\"" + tmp.str() + "\"^^" +
                  osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeTag(const std::string& s,
                                       const osm2ttl::osm::Tag& tag) {
  const std::string& key = tag.first;
  const std::string& value = tag.second;
  if (key == "admin_level") {
    writeTriple(s,
                generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, key),
                generateLiteral(
                    value, "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
  } else {
    writeTriple(s,
                generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, key),
                generateLiteral(value, ""));
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeTag(const std::string& s,
                                       const osmium::Tag& tag) {
  std::string_view key{tag.key()};
  std::string_view value{tag.value()};
  if (key == "admin_level") {
    writeTriple(s,
                generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, key),
                generateLiteral(
                    value, "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
  } else {
    writeTriple(s,
                generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, key),
                generateLiteral(value, ""));
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeTagList(const std::string& s,
                                           const osm2ttl::osm::TagList& tags) {
  for (const auto& tag : tags) {
    writeTag(s, tag);
    const std::string& key = tag.first;
    std::string value = tag.second;
    if (!_config.skipWikiLinks) {
      if (key == "wikidata") {
        // Only take first wikidata entry if ; is found
        auto end = value.find(';');
        if (end != std::string::npos) {
          value = value.erase(end);
        }
        // Remove all but Q and digits to ensure Qdddddd format
        value.erase(
            remove_if(value.begin(), value.end(),
                      [](char c) { return (c != 'Q' && isdigit(c) == 0); }),
            value.end());

        writeTriple(
            s, generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, key),
            generateIRI(osm2ttl::ttl::constants::NAMESPACE__WIKIDATA_ENTITY,
                        value));
      }
      if (key == "wikipedia") {
        auto pos = value.find(':');
        if (pos != std::string::npos) {
          std::string lang = value.substr(0, pos);
          std::string entry = value.substr(pos + 1);
          writeTriple(
              s, osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA,
              generateIRI("https://" + lang + ".wikipedia.org/wiki/", entry));
        } else {
          writeTriple(s, osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA,
                      generateIRI("https://www.wikipedia.org/wiki/", value));
        }
      }
    }
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeTagList(const std::string& s,
                                           const osmium::TagList& tags) {
  for (const auto& tag : tags) {
    writeTag(s, tag);
    std::string_view key{tag.key()};
    if (!_config.skipWikiLinks) {
      if (key == "wikidata") {
        // Only take first wikidata entry if ; is found
        std::string value{tag.value()};
        auto end = value.find(';');
        if (end != std::string::npos) {
          value = value.erase(end);
        }
        // Remove all but Q and digits to ensure Qdddddd format
        value.erase(
            remove_if(value.begin(), value.end(),
                      [](char c) { return (c != 'Q' && isdigit(c) == 0); }),
            value.end());

        writeTriple(
            s, generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, key),
            generateIRI(osm2ttl::ttl::constants::NAMESPACE__WIKIDATA_ENTITY,
                        value));
      }
      if (key == "wikipedia") {
        std::string_view value{tag.value()};
        auto pos = value.find(':');
        if (pos != std::string::npos) {
          std::string lang{value.substr(0, pos)};
          std::string_view entry = value.substr(pos + 1);
          writeTriple(
              s, osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA,
              generateIRI("https://" + lang + ".wikipedia.org/wiki/", entry));
        } else {
          writeTriple(s, osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA,
                      generateIRI("https://www.wikipedia.org/wiki/", value));
        }
      }
    }
  }
}

// ____________________________________________________________________________
template <>
std::string osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>::formatIRI(
    std::string_view p, std::string_view v) {
  // NT:  [8]    IRIREF
  //      https://www.w3.org/TR/n-triples/#grammar-production-IRIREF
  auto prefix = _prefixes.find(std::string{p});
  if (prefix != _prefixes.end()) {
    return IRIREF(prefix->second, v);
  }
  return IRIREF(p, v);
}
// ____________________________________________________________________________
template <>
std::string osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>::formatIRI(
    std::string_view p, std::string_view v) {
  // TTL: [135s] iri
  //      https://www.w3.org/TR/turtle/#grammar-production-iri
  //      [18]   IRIREF (same as NT)
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  //      [136s] PrefixedName
  //      https://www.w3.org/TR/turtle/#grammar-production-PrefixedName
  auto prefix = _prefixes.find(std::string{p});
  // If known prefix -> PrefixedName
  if (prefix != _prefixes.end()) {
    return PrefixedName(p, v);
  }
  return IRIREF(p, v);
}

// ____________________________________________________________________________
template <>
std::string osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::formatIRI(
    std::string_view p, std::string_view v) {
  // TTL: [135s] iri
  //      https://www.w3.org/TR/turtle/#grammar-production-iri
  //      [18]   IRIREF (same as NT)
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  //      [136s] PrefixedName
  //      https://www.w3.org/TR/turtle/#grammar-production-PrefixedName
  auto prefix = _prefixes.find(std::string{p});
  // If known prefix -> PrefixedName
  if (prefix != _prefixes.end()) {
    return PrefixedName(p, v);
  }
  return IRIREF(p, v);
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::IRIREF(std::string_view p,
                                            std::string_view v) {
  return "<" + encodeIRIREF(p) + encodeIRIREF(v) + ">";
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::PrefixedName(std::string_view p,
                                                  std::string_view v) {
  return std::string(p) + ":" + encodePN_LOCAL(v);
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::STRING_LITERAL_QUOTE(std::string_view s) {
  // NT:  [9]   STRING_LITERAL_QUOTE
  //      https://www.w3.org/TR/n-triples/#grammar-production-STRING_LITERAL_QUOTE
  // TTL: [22]  STRING_LITERAL_QUOTE
  //      https://www.w3.org/TR/turtle/#grammar-production-STRING_LITERAL_QUOTE
  std::string tmp;
  tmp.reserve(s.size() * 2);
  tmp += "\"";
  for (const auto c : s) {
    switch (c) {
      case '\"':  // #x22
        tmp += "\\\"";
        break;
      case '\\':  // #x5C
        tmp += "\\\\";
        break;
      case '\n':  // #x0A
        tmp += "\\n";
        break;
      case '\r':  // #x0D
        tmp += "\\r";
        break;
      default:
        tmp += c;
    }
  }
  tmp += "\"";
  return tmp;
}

// ____________________________________________________________________________
template <typename T>
uint8_t osm2ttl::ttl::Writer<T>::utf8Length(char c) {
  if ((c & k0x80) == 0) {
    return k1Byte;
  }
  if ((c & k0xE0) == k0xC0) {
    return k2Byte;
  }
  if ((c & k0xF0) == k0xE0) {
    return k3Byte;
  }
  if ((c & k0xF8) == k0xF0) {
    return k4Byte;
  }
  throw std::domain_error("Invalid UTF-8 Sequence start " + std::to_string(c));
}

// ____________________________________________________________________________
template <typename T>
uint8_t osm2ttl::ttl::Writer<T>::utf8Length(std::string_view s) {
  return utf8Length(s[0]);
}

// ____________________________________________________________________________
template <typename T>
uint32_t osm2ttl::ttl::Writer<T>::utf8Codepoint(std::string_view s) {
  switch (utf8Length(s)) {
    case k4Byte:
      // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      //      111   111111   111111   111111 = 7 3F 3F 3F
      return (((s[0] & k0x07) << UTF8_CODEPOINT_OFFSET_BYTE4) |
              ((s[1] & k0x3F) << UTF8_CODEPOINT_OFFSET_BYTE3) |
              ((s[2] & k0x3F) << UTF8_CODEPOINT_OFFSET_BYTE2) | (s[3] & k0x3F));
    case k3Byte:
      // 1110xxxx 10xxxxxx 10xxxxxx
      //     1111   111111   111111 = F 3F 3F
      return (((s[0] & k0x0F) << UTF8_CODEPOINT_OFFSET_BYTE3) |
              ((s[1] & k0x3F) << UTF8_CODEPOINT_OFFSET_BYTE2) | (s[2] & k0x3F));
    case k2Byte:
      // 110xxxxx 10xxxxxx
      //    11111   111111 = 1F 3F
      return (((s[0] & k0x1F) << UTF8_CODEPOINT_OFFSET_BYTE2) | (s[1] & k0x3F));
    case k1Byte:
      // 0xxxxxxx
      //  1111111 = 7F
      return (s[0] & k0x7F);
    default:
      throw std::domain_error("Invalid UTF-8 Sequence: " + std::string{s});
  }
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::UCHAR(char c) {
  // NT:  [10]  UCHAR
  //      https://www.w3.org/TR/n-triples/#grammar-production-UCHAR
  // TTL: [26]  UCHAR
  //      https://www.w3.org/TR/turtle/#grammar-production-UCHAR
  std::ostringstream tmp;
  tmp << "\\u00" << std::hex << ((c & k0xF0) >> 4) << std::hex << (c & k0x0F);
  return tmp.str();
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::encodeIRIREF(std::string_view s) {
  // NT:  [8]   IRIREF
  //      https://www.w3.org/TR/n-triples/#grammar-production-IRIREF
  // TTL: [18]  IRIREF
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  std::string tmp;
  tmp.reserve(s.size() * 2);
  for (size_t pos = 0; pos < s.size(); ++pos) {
    // Force non-allowed chars to UCHAR
    auto c = s[pos];
    if ((s[pos] >= 0 && c <= ' ') || c == '<' || c == '>' || c == '{' ||
        c == '}' || c == '\"' || c == '|' || c == '^' || c == '`' ||
        c == '\\') {
      tmp += UCHAR(s[pos]);
      continue;
    }
    uint8_t length = utf8Length(s[pos]);
    tmp += s.substr(pos, length);
    pos += length - 1;
  }
  return tmp;
}

// ____________________________________________________________________________
template <>
std::string osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>::encodeIRIREF(
    std::string_view s) {
  // NT:  [8]   IRIREF
  //      https://www.w3.org/TR/n-triples/#grammar-production-IRIREF
  // TTL: [18]  IRIREF
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  std::string tmp;
  tmp.reserve(s.size() * 2);
  for (size_t pos = 0; pos < s.size(); ++pos) {
    uint8_t length = utf8Length(s[pos]);
    // Force non-allowed chars to UCHAR
    if (length == k1Byte) {
      if ((s[pos] >= 0 && s[pos] <= ' ') || s[pos] == '<' || s[pos] == '>' ||
          s[pos] == '{' || s[pos] == '}' || s[pos] == '\"' || s[pos] == '|' ||
          s[pos] == '^' || s[pos] == '`' || s[pos] == '\\') {
        tmp += encodePERCENT(s.substr(pos, 1));
        continue;
      }
    }
    tmp += s.substr(pos, length);
    pos += length - 1;
  }
  return tmp;
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::encodePERCENT(std::string_view s) {
  // TTL: [170s] PERCENT
  //      https://www.w3.org/TR/turtle/#grammar-production-PERCENT
  std::ostringstream tmp;
  uint32_t c = utf8Codepoint(s);
  uint32_t mask = BITS_OF_BYTE;
  bool echo = false;
  for (int shift = 3; shift >= 0; --shift) {
    uint8_t v = (c & (mask << (shift * BIT_IN_BYTE)) >> shift);
    echo |= (v > 0);
    if (!echo) {
      continue;
    }
    tmp << "%" << std::hex << ((v & k0xF0) >> BIT_IN_NIBBLE) << std::hex
        << (v & k0x0F);
  }
  return tmp.str();
}

// ____________________________________________________________________________
template <typename T>
std::string osm2ttl::ttl::Writer<T>::encodePN_LOCAL(std::string_view s) {
  // TTL: [168s] PN_LOCAL
  //      https://www.w3.org/TR/turtle/#grammar-production-PN_LOCAL
  std::string tmp;
  tmp.reserve(s.size() * 2);
  for (size_t pos = 0; pos < s.size(); ++pos) {
    // PN_LOCAL      ::= (PN_CHARS_U | ':' | [0-9] | PLX)
    //                   ((PN_CHARS | '.' | ':' | PLX)*
    //                   (PN_CHARS | ':' | PLX))?
    //
    // PN_CHARS_U    ::= PN_CHARS_BASE | '_'
    //
    // PN_CHARS      ::= PN_CHARS_U | '-' | [0-9] | #x00B7 | [#x0300-#x036F] |
    //                   [#x203F-#x2040]
    //
    // PN_CHARS_BASE ::= [A-Z] | [a-z] | [#x00C0-#x00D6] | [#x00D8-#x00F6] |
    //                   [#x00F8-#x02FF] | [#x0370-#x037D] | [#x037F-#x1FFF] |
    //                   [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] |
    //                   [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] |
    //                   [#x10000-#xEFFFF]
    //
    // PLX           ::= PERCENT | PN_LOCAL_ESC
    //
    // PERCENT       ::= '%' HEX HEX
    //
    // HEX           ::= [0-9] | [A-F] | [a-f]
    //
    // PN_LOCAL_ESC  ::= '\' ('_' | '~' | '.' | '-' | '!' | '$' | '&' | "'" |
    //                        '(' | ')' | '*' | '+' | ',' | ';' | '=' | '/' |
    //                        '?' | '#' | '@' | '%')

    auto currentChar = s[pos];
    // _, :, A-Z, a-z, and 0-9 always allowed:
    if (currentChar == ':' || currentChar == '_' ||
        (currentChar >= 'A' && currentChar <= 'Z') ||
        (currentChar >= 'a' && currentChar <= 'z') ||
        (currentChar >= '0' && currentChar <= '9')) {
      tmp += currentChar;
      continue;
    }
    // First and last char is never .
    if (currentChar == '.' && pos > 0 && pos < s.size() - 1) {
      tmp += currentChar;
      continue;
    }
    // First char is nevvr -
    if (currentChar == '-' && pos > 0) {
      tmp += currentChar;
      continue;
    }
    // Handle PN_LOCAL_ESC
    if (currentChar == '!' || (currentChar >= '#' && currentChar <= '@') ||
        currentChar == ';' || currentChar == '=' || currentChar == '?' ||
        currentChar == '~') {
      tmp += '\\' + currentChar;
      continue;
    }
    uint8_t length = utf8Length(currentChar);
    std::string_view sub = s.substr(pos, length);
    uint32_t c = utf8Codepoint(sub);
    // Handle allowed Codepoints for CHARS_U
    if ((c >= k0xC0 && c <= k0xD6) || (c >= k0xD8 && c <= k0xF6) ||
        (c >= k0xF8 && c <= k0x2FF) || (c >= k0x370 && c <= k0x37D) ||
        (c >= k0x37F && c <= k0x1FFF) || (c >= k0x200C && c <= k0x200D) ||
        (c >= k0x2070 && c <= k0x218F) || (c >= k0x2C00 && c <= k0x2FEF) ||
        (c >= k0x3001 && c <= k0xD7FF) || (c >= k0xF900 && c <= k0xFDCF) ||
        (c >= k0xFDF0 && c <= k0xFFFD) || (c >= k0x10000 && c <= k0xEFFFF)) {
      tmp += sub;
    } else if (pos > 0 && (c == k0xB7 || (c >= k0x300 && c <= k0x36F) ||
                           (c >= k0x203F && c <= k0x2040))) {
      tmp += sub;
    } else {
      // Escape all other symbols
      tmp += encodePERCENT(sub);
    }
    // Shift new pos according to utf8-bytecount
    pos += length - 1;
  }
  return tmp;
}

// ____________________________________________________________________________
template <typename T>
uint64_t osm2ttl::ttl::Writer<T>::_blankNodeCounter;
template class osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>;
template class osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>;
template class osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>;