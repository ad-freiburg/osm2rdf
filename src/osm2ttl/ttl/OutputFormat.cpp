// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/OutputFormat.h"

#include <iostream>
#include <string>
#include <sstream>

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/LangTag.h"
#include "osm2ttl/ttl/Literal.h"

/**
 * RDF 1.1 N-Triples
 * https://www.w3.org/TR/n-triples/#n-triples-grammar
 *
 *  RDF 1.1 Turtle
 * https://www.w3.org/TR/turtle/#sec-grammar-grammar
 */

// ____________________________________________________________________________
osm2ttl::ttl::OutputFormat::OutputFormat() {
  initPrefixes();
}

// ____________________________________________________________________________
osm2ttl::ttl::OutputFormat::OutputFormat(Value v) : _value(v) {
  initPrefixes();
}

// ____________________________________________________________________________
void osm2ttl::ttl::OutputFormat::initPrefixes() {
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
std::string osm2ttl::ttl::OutputFormat::header() const {
  if (_value == osm2ttl::ttl::OutputFormat::NT) {
    return "";
  }
  std::stringstream tmp;
  for (const auto& p : _prefixes) {
    tmp << "@prefix " << p.first << ": <" << p.second << "> .\n";
  }
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::format(
  const osm2ttl::ttl::BlankNode& b) const {
  // NT:  [141s] BLANK_NODE_LABEL
  // TTL: [141s] BLANK_NODE_LABEL
  //      [162s] ANON (not used here)
  std::stringstream tmp;
  tmp << "_:" << b.getId();
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::format(
  const osm2ttl::ttl::LangTag& l) const {
  // NT:  [144s] LANGTAG
  //      https://www.w3.org/TR/n-triples/#grammar-production-LANGTAG
  // TTL: [144s] LANGTAG
  //      https://www.w3.org/TR/turtle/#grammar-production-LANGTAG
  std::stringstream tmp;
  tmp << "@";
  std::string_view s = l.value();
  size_t partCount = 0;
  for (size_t pos = 0; pos < s.length(); pos++) {
    if (pos == 0 && s[0] == '-') {
      throw;
    }
    if ((s[pos] >= 'a' && s[pos] <= 'z') ||
        (s[pos] >= 'A' && s[pos] <= 'Z') ||
        (s[pos] >= '0' && s[pos] <= '9' && partCount > 0)) {
      tmp << s[pos];
    } else {
      throw;
    }
  }
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::format(
  const osm2ttl::ttl::Literal& l) const {
  // NT:  [6]    literal
  //      https://www.w3.org/TR/n-triples/#grammar-production-literal
  //      [9]    STRING_LITERAL_QUOTE
  // TTL: [13]   literal
  //      https://www.w3.org/TR/turtle/#grammar-production-literal
  //      Currently only:
  //      [128s] RDFLiteral
  //      https://www.w3.org/TR/turtle/#grammar-production-RDFLiteral
  std::stringstream tmp;
  tmp << osm2ttl::ttl::OutputFormat::STRING_LITERAL_QUOTE(l.value());
  if (auto iri = l.iri()) {
    tmp << "^^" << format(*iri);
  }
  if (auto langTag = l.langTag()) {
    tmp << osm2ttl::ttl::OutputFormat::format(*langTag);
  }
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::format(
  const osm2ttl::ttl::IRI& i) const {
  // NT:  [8]    IRIREF
  //      https://www.w3.org/TR/n-triples/#grammar-production-IRIREF
  // TTL: [135s] iri
  //      https://www.w3.org/TR/turtle/#grammar-production-iri
  //      [18]   IRIREF (same as NT)
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  //      [136s] PrefixedName
  //      https://www.w3.org/TR/turtle/#grammar-production-PrefixedName
  switch (_value) {
  case osm2ttl::ttl::OutputFormat::NT:
    // Expand prefix
    if (_prefixes.find(i.prefix()) != _prefixes.end()) {
      return IRIREF(_prefixes.at(i.prefix()), i.value());
    }
    return IRIREF(i.prefix(), i.value());
  case osm2ttl::ttl::OutputFormat::TTL:
    // If known prefix -> PrefixedName
    if (_prefixes.find(i.prefix()) != _prefixes.end()) {
      return PrefixedName(i.prefix(), i.value());
    }
    return IRIREF(i.prefix(), i.value());
  default:
    throw;
  }
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::IRIREF(const std::string& p,
                                               const std::string& v) {
  return "<" + iriEncode(p) + iriEncode(v) + ">";
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::PrefixedName(const std::string& p,
                                                     const std::string& v) {
  std::stringstream tmp;
  if (!p.empty()) {
    tmp << p << ":";
  }
  tmp << v;
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::STRING_LITERAL_QUOTE(
  const std::string& s) {
  // NT:  [9]   STRING_LITERAL_QUOTE
  //      https://www.w3.org/TR/n-triples/#grammar-production-STRING_LITERAL_QUOTE
  // TTL: [22]  STRING_LITERAL_QUOTE
  //      https://www.w3.org/TR/turtle/#grammar-production-STRING_LITERAL_QUOTE
  std::stringstream tmp;
  tmp << "\"";
  for (size_t pos = 0; pos < s.size(); ++pos) {
    switch (s[pos]) {
      case '\"':  // #x22
        tmp << ECHAR(s[pos]);
        break;
      case '\\':  // #x5C
        tmp << ECHAR(s[pos]);
        break;
      case '\n':  // #x0A
        tmp << ECHAR(s[pos]);
        break;
      case '\r':  // #x0D
        tmp << ECHAR(s[pos]);
        break;
      default:
        tmp << s[pos];
    }
  }
  tmp << "\"";
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::STRING_LITERAL_SINGLE_QUOTE(
  const std::string& s) {
  // TTL: [23]  STRING_LITERAL_SINGLE_QUOTE
  //      https://www.w3.org/TR/turtle/#grammar-production-STRING_LITERAL_SINGLE_QUOTE
  std::stringstream tmp;
  tmp << "\"";
  for (size_t pos = 0; pos < s.size(); ++pos) {
    switch (s[pos]) {
      case '\'':  // #x27
        tmp << ECHAR(s[pos]);
        break;
      case '\\':  // #x5C
        tmp << ECHAR(s[pos]);
        break;
      case '\n':  // #x0A
        tmp << ECHAR(s[pos]);
        break;
      case '\r':  // #x0D
        tmp << ECHAR(s[pos]);
        break;
      default:
        tmp << s[pos];
    }
  }
  tmp << "\"";
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::ECHAR(char c) {
  // NT:  [153s] ECHAR
  //      https://www.w3.org/TR/n-triples/#grammar-production-ECHAR
  // TTL: [159s] ECHAR
  //      https://www.w3.org/TR/turtle/#grammar-production-ECHAR
  switch (c) {
  case '\t':
    return "\\t";
  case '\b':
    return "\\b";
  case '\n':
    return "\\n";
  case '\r':
    return "\\r";
  case '\f':
    return "\\f";
  case '\"':
    return "\\\"";
  case '\'':
    return "\\'";
  case '\\':
    return "\\\\";
  default:
    return std::string(1, c);
  }
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::iriEncode(std::string_view s) {
  // NT:  [8]    IRIREF
  //      https://www.w3.org/TR/n-triples/#grammar-production-IRIREF
  // TTL: [18]   IRIREF
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  // TODO(lehmanna): make this compliant to defs.
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

