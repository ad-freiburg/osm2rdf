// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/Writer.h"

#include <osm2ttl/ttl/Constants.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "boost/iostreams/filter/bzip2.hpp"
#include "osm2ttl/config/Config.h"
#include "osmium/osm/item_type.hpp"

static const int k0xB7 = 0xB7;
// ____________________________________________________________________________
template <typename T>
osm2ttl::ttl::Writer<T>::Writer(const osm2ttl::config::Config& config)
    : _config(config) {
  // Generate constants
  osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__GEOSPARQL, "hasGeometry");
  osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__GEOSPARQL, "wktLiteral");
  osm2ttl::ttl::constants::IRI__OGC_CONTAINS =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "contains");
  osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "contained_by");
  osm2ttl::ttl::constants::IRI__OGC_CONTAINS_AREA =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "contains_area");
  osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY_AREA = generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "contained_by_area");
  osm2ttl::ttl::constants::IRI__OGC_INTERSECTS =
      generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "intersects");
  osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY = generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "intersected_by");
  osm2ttl::ttl::constants::IRI__OGC_INTERSECTS_AREA = generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "intersects_area");
  osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY_AREA = generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "intersected_by_area");
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
  if (_config.outputCompress) {
    _out.push(boost::iostreams::bzip2_compressor{});
  }
  if (!_config.output.empty()) {
    _outFile.open(_config.output);
    _out.push(_outFile);
    return _outFile.is_open();
  }
  _out.push(std::cout);
  return true;
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::close() {
  _out.pop();
  if (_outFile.is_open()) {
    _outFile.close();
  }
}

// ____________________________________________________________________________
template <typename T>
void osm2ttl::ttl::Writer<T>::writeHeader() {
  for (const auto& [prefix, iriref] : _prefixes) {
    writeTriple("@prefix", prefix + ":", "<" + iriref + ">");
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
#pragma omp critical(writeTriple)
  { _out << s + " " + p + " " + o + " .\n"; }
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
