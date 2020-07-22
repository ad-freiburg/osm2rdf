// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/OutputFormat.h"

#include <iomanip>
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
  _prefixes["rdf"] = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
  _prefixes["rdfs"] = "http://www.w3.org/2000/01/rdf-schema#";
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
  std::ostringstream tmp;
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
  std::ostringstream tmp;
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
  std::ostringstream tmp;
  tmp << "@";
  std::string_view s = l.value();
  bool allowDigits = false;
  for (size_t pos = 0; pos < s.length(); pos++) {
    if (pos == 0 && s[0] == '-') {
      throw std::domain_error("Invalid LangTag " + l.value());
    }
    if ((s[pos] >= 'a' && s[pos] <= 'z') ||
        (s[pos] >= 'A' && s[pos] <= 'Z') ||
        (s[pos] >= '0' && s[pos] <= '9' && allowDigits)) {
      tmp << s[pos];
    } else if (s[pos] == '-') {
      allowDigits = true;
      tmp << s[pos];
    } else {
      throw std::domain_error("Invalid LangTag " + l.value());
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
  std::string s = osm2ttl::ttl::OutputFormat::STRING_LITERAL_QUOTE(l.value());
  if (auto iri = l.iri()) {
    return s += "^^" + format(*iri);
  }
  if (auto langTag = l.langTag()) {
    return s += osm2ttl::ttl::OutputFormat::format(*langTag);
  }
  return s;
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
  auto prefix = _prefixes.find(i.prefix());
  switch (_value) {
  case osm2ttl::ttl::OutputFormat::NT:
    // Expand prefix
    if (prefix != _prefixes.end()) {
      return IRIREF(prefix->second, i.value());
    }
    return IRIREF(i.prefix(), i.value());
  case osm2ttl::ttl::OutputFormat::TTL:
    [[fallthrough]];
  case osm2ttl::ttl::OutputFormat::QLEVER:
    // If known prefix -> PrefixedName
    if (prefix != _prefixes.end()) {
      return PrefixedName(i.prefix(), i.value());
    }
    return IRIREF(i.prefix(), i.value());
  default:
    throw std::domain_error("Invalid OutputFormat " + std::to_string(_value));
  }
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::IRIREF(const std::string& p,
                                               const std::string& v) const {
  return "<" + encodeIRIREF(p) + encodeIRIREF(v) + ">";
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::PrefixedName(const std::string& p,
                                                     const std::string& v)
  const {
  return p + ":" + encodePN_LOCAL(v);
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::STRING_LITERAL_QUOTE(
  const std::string& s) const {
  // NT:  [9]   STRING_LITERAL_QUOTE
  //      https://www.w3.org/TR/n-triples/#grammar-production-STRING_LITERAL_QUOTE
  // TTL: [22]  STRING_LITERAL_QUOTE
  //      https://www.w3.org/TR/turtle/#grammar-production-STRING_LITERAL_QUOTE
  std::ostringstream tmp;
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
  const std::string& s) const {
  // TTL: [23]  STRING_LITERAL_SINGLE_QUOTE
  //      https://www.w3.org/TR/turtle/#grammar-production-STRING_LITERAL_SINGLE_QUOTE
  std::ostringstream tmp;
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
std::string osm2ttl::ttl::OutputFormat::ECHAR(char c) const {
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
uint8_t osm2ttl::ttl::OutputFormat::utf8Length(char c) const {
  if ((c & 0xF8) == 0xF0) { return 4; }
  if ((c & 0xF0) == 0xE0) { return 3; }
  if ((c & 0xE0) == 0xC0) { return 2; }
  if ((c & 0x80) == 0x00) { return 1; }
  throw std::domain_error("Invalid UTF-8 Sequence start " + std::to_string(c));
}

// ____________________________________________________________________________
uint8_t osm2ttl::ttl::OutputFormat::utf8Length(const std::string& s) const {
  return utf8Length(s[0]);
}

// ____________________________________________________________________________
uint32_t osm2ttl::ttl::OutputFormat::utf8Codepoint(const std::string& s)
  const {
  switch (utf8Length(s)) {
    case 4:
      // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      //      111   111111   111111   111111 = 7 3F 3F 3F
      return (((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12)
        | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F));
      break;
    case 3:
      // 1110xxxx 10xxxxxx 10xxxxxx
      //     1111   111111   111111 = F 3F 3F
      return (((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F));
    case 2:
      // 110xxxxx 10xxxxxx
      //    11111   111111 = 1F 3F
      return (((s[0] & 0x1F) << 6) | (s[1] & 0x3F));
    case 1:
      // 0xxxxxxx
      //  1111111 = 7F
      return (s[0] & 0x7F);
    default:
      throw std::domain_error("Invalid UTF-8 Sequence: " + s);
  }
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::UCHAR(char c)
  const {
  // NT:  [10]  UCHAR
  //      https://www.w3.org/TR/n-triples/#grammar-production-UCHAR
  // TTL: [26]  UCHAR
  //      https://www.w3.org/TR/turtle/#grammar-production-UCHAR
  std::ostringstream tmp;
  tmp << "\\u00" << std::hex << ((c & 0xF0) >> 4) << std::hex << (c & 0x0F);
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::UCHAR(const std::string& s)
  const {
  // NT:  [10]  UCHAR
  //      https://www.w3.org/TR/n-triples/#grammar-production-UCHAR
  // TTL: [26]  UCHAR
  //      https://www.w3.org/TR/turtle/#grammar-production-UCHAR
  std::ostringstream tmp;
  uint32_t c = utf8Codepoint(s);
  if (c <= 0xFFFF) {
    tmp << "\\u" << std::setfill('0') << std::setw(4);
  } else {
    tmp << "\\U" << std::setfill('0') << std::setw(8);
  }
  tmp << std::right << std::hex << c;
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::encodeIRIREF(const std::string& s)
  const {
  // NT:  [8]   IRIREF
  //      https://www.w3.org/TR/n-triples/#grammar-production-IRIREF
  // TTL: [18]  IRIREF
  //      https://www.w3.org/TR/turtle/#grammar-production-IRIREF
  std::ostringstream tmp;
  for (size_t pos = 0; pos < s.size(); ++pos) {
    uint8_t length = utf8Length(s[pos]);
    // Force non-allowed chars to UCHAR
    if (length == 1) {
      if ((s[pos] >= 0 && s[pos] <= ' ') ||
          s[pos] == '<' || s[pos] == '>' ||
          s[pos] == '{' || s[pos] == '}' ||
          s[pos] == '\"' || s[pos] == '|' ||
          s[pos] == '^' || s[pos] == '`' ||
          s[pos] == '\\') {
        // TODO(lehmanna): CHeck what to do... %-encoding not explicitly allowed
        // in grammer... but in IRI
        // RFC: https://tools.ietf.org/html/rfc3987#section-2.2
        if (_value == osm2ttl::ttl::OutputFormat::QLEVER) {
          tmp << encodePERCENT(s.substr(pos, 1));
        } else {
          tmp << UCHAR(s[pos]);
        }
        continue;
      }
    }
    tmp << s.substr(pos, length);
    pos += length-1;
  }
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::encodePERCENT(const std::string& s)
  const {
  // TTL: [170s] PERCENT
  //      https://www.w3.org/TR/turtle/#grammar-production-PERCENT
  std::ostringstream tmp;
  uint32_t c = utf8Codepoint(s);
  uint32_t mask = 0xFF;
  bool echo = false;
  for (int shift = 3; shift >= 0; --shift) {
    uint8_t v = (c & (mask << (shift * 8)) >> shift);
    echo |= (v > 0);
    if (!echo) {
      continue;
    }
    tmp << "%"<< std::hex << ((v & 0xF0) >> 4) << std::hex << (v & 0x0F);
  }
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::OutputFormat::encodePN_LOCAL(const std::string& s)
  const {
  // TTL: [168s] PN_LOCAL
  //      https://www.w3.org/TR/turtle/#grammar-production-PN_LOCAL
  std::ostringstream tmp;
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

    // _, :, A-Z, a-z, and 0-9 always allowed:
    if (s[pos] == ':' || s[pos] == '_' ||
        (s[pos] >= 'A' && s[pos] <= 'Z') ||
        (s[pos] >= 'a' && s[pos] <= 'z') ||
        (s[pos] >= '0' && s[pos] <= '9')) {
      tmp << s[pos];
      continue;
    }
    // First and last char is never .
    if (pos > 0 && pos < s.size() - 1 && s[pos] == '.') {
      tmp << s[pos];
      continue;
    }
    // Handle PN_LOCAL_ESC
    if (s[pos] == '_' || s[pos] == '~' || s[pos] == '.' || s[pos] == '-' ||
        s[pos] == '!' || s[pos] == '$' || s[pos] == '&' || s[pos] == '\'' ||
        s[pos] == '(' || s[pos] == ')' || s[pos] == '*' || s[pos] == '+' ||
        s[pos] == ',' || s[pos] == ';' || s[pos] == '=' || s[pos] == '/' ||
        s[pos] == '?' || s[pos] == '#' || s[pos] == '@' || s[pos] == '%') {
      tmp << '\\' << s[pos];
      continue;
    }
    uint8_t length = utf8Length(s[pos]);
    std::string sub = s.substr(pos, length);
    uint32_t c = utf8Codepoint(sub);
    // Handle allowed Codepoints for CHARS_U
    if ((c >= 0xC0 && c <= 0xD6) || (c >= 0xD8 && c <= 0xF6) ||
        (c >= 0xF8 && c <= 0x2FF) || (c >= 0x370 && c <= 0x37D) ||
        (c >= 0x37F && c <= 0x1FFF) || (c >= 0x200C && c <= 0x200D) ||
        (c >= 0x2070 && c <= 0x218F) || (c >= 0x2C00 && c <= 0x2FEF) ||
        (c >= 0x3001 && c <= 0xD7FF) || (c >= 0xF900 && c <= 0xFDCF) ||
        (c >= 0xFDF0 && c <= 0xFFFD) || (c >= 0x10000 && c <= 0xEFFFF)) {
      tmp << sub;
    } else if (pos > 0 && (c == 0xB7 || (c >= 0x300 && c <= 0x36F) ||
                           (c >= 0x203F && c <= 0x2040))) {
      tmp << sub;
    } else {
      // Escape all other symbols
      tmp << encodePERCENT(sub);
    }
    // Shift new pos according to utf8-bytecount
    pos += length-1;
  }
  return tmp.str();
}
