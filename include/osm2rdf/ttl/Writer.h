// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2RDF_TTL_WRITER_H_
#define OSM2RDF_TTL_WRITER_H_

static const int k1Byte = 1;
static const int k2Byte = 2;
static const int k3Byte = 3;
static const int k4Byte = 4;

static const int k0x07 = 0x07;
static const int k0x0F = 0x0F;
static const int k0x1F = 0x1F;
static const int k0x3F = 0x3F;
static const int k0x7F = 0x7F;
static const int k0x80 = 0x80;
static const int k0xB7 = 0xB7;
static const int k0xC0 = 0xC0;
static const int k0xD6 = 0xD6;
static const int k0xD8 = 0xD8;
static const int k0xE0 = 0xE0;
static const int k0xF0 = 0xF0;
static const int k0xF6 = 0xF6;
static const int k0xF8 = 0xF8;
static const int k0xFF = 0xFF;
static const unsigned int k0xFFU = 0xFFU;
static const int k0x2FF = 0x2FF;
static const int k0x370 = 0x370;
static const int k0x37D = 0x37D;
static const int k0x203F = 0x203F;
static const int k0x2040 = 0x2040;
static const int k0x36F = 0x36F;
static const int k0x300 = 0x300;
static const int k0x37F = 0x37F;
static const int k0x1FFF = 0x1FFF;
static const int k0x200C = 0x200C;
static const int k0x200D = 0x200D;
static const int k0x2070 = 0x2070;
static const int k0x218F = 0x218F;
static const int k0x2C00 = 0x2C00;
static const int k0x2FEF = 0x2FEF;
static const int k0x3001 = 0x3001;
static const int k0xD7FF = 0xD7FF;
static const int k0xF900 = 0xF900;
static const int k0xFDCF = 0xFDCF;
static const int k0xFDF0 = 0xFDF0;
static const int k0xFFFD = 0xFFFD;
static const unsigned int k0xFFFFU = 0xFFFFU;
static const int k0x10000 = 0x10000;
static const int k0xEFFFF = 0xEFFFF;

static const int UTF8_CODEPOINT_OFFSET_BYTE4 = 18;
static const int UTF8_CODEPOINT_OFFSET_BYTE3 = 12;
static const int UTF8_CODEPOINT_OFFSET_BYTE2 = 6;
static const int NUM_BITS_IN_BYTE = 8;
static const int NUM_BITS_IN_NIBBLE = 4;
static const int MASK_BITS_OF_ONE_BYTE = k0xFF;

static const int UTF8_BYTES_LONG = 8;
static const int UTF8_BYTES_SHORT = 4;
#include <string>

#include "gtest/gtest_prod.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Box.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/Tag.h"
#include "osm2rdf/osm/TagList.h"
#include "osm2rdf/osm/Way.h"
#include "osm2rdf/ttl/Constants.h"
#include "osm2rdf/util/Output.h"

namespace osm2rdf::ttl {

template <typename T>
class Writer {
 public:
  Writer(const osm2rdf::config::Config& config, osm2rdf::util::Output* output);
  ~Writer();

  // Write statistic json into output.
  void writeStatisticJson(const std::filesystem::path& output);

  // Write the header (does nothing for NT)
  void writeHeader();

  // Write a single RDF line. The contents of s, p, and o are not checked.
  void writeTriple(const std::string& s, const std::string& p,
                   const std::string& o);

  // addPrefix adds the given prefix and value. If the prefix already exists
  // false is returned.
  bool addPrefix(const std::string& prefix, std::string_view value);
  // resolvePrefix resolves the given prefix. If the prefix is unknown it is
  // returned as provided.
  std::string resolvePrefix(std::string_view p);

  // generateBlankNode creates a new unique identifier for a blank node.
  std::string generateBlankNode();

  // generateIRI creates a IRI from given prefix p and string value v.
  // Assumes that both p and v are "safe", that is, they can be used
  // directly in the TTL
  std::string generateIRIUnsafe(std::string_view p, std::string_view v);

  // generateIRI creates a IRI from given prefix p and ID value v.
  std::string generateIRI(std::string_view p, uint64_t v);
  // generateIRI creates a IRI from given prefix p and string value v.
  std::string generateIRI(std::string_view p, std::string_view v);

  // generateLangTag creates a LangTag from the given string.
  std::string generateLangTag(std::string_view s);
  // generateLangTag creates a Literal from the given string value v.
  // If suffix s is not empty, it will be appended as is.
  std::string generateLiteral(std::string_view v, std::string_view s);

  // Assumes that both p and v are "safe", that is, they can be used
  // directly in the TTL
  std::string generateLiteralUnsafe(std::string_view v, std::string_view s);

  // -------------------------------------------------------------------------
  // Following functions are used by the ones above. These functions implement
  // the grammars.
  // -------------------------------------------------------------------------
  std::string formatIRI(std::string_view p, std::string_view v);
  std::string formatIRIUnsafe(std::string_view p, std::string_view v);

  std::string STRING_LITERAL_QUOTE(std::string_view s);
  FRIEND_TEST(WriterGrammarNT, RULE_9_STRING_LITERAL_QUOTE);
  FRIEND_TEST(WriterGrammarTTL, RULE_22_STRING_LITERAL_QUOTE);

  std::string STRING_LITERAL_SINGLE_QUOTE(std::string_view s);
  FRIEND_TEST(WriterGrammarTTL, RULE_23_STRING_LITERAL_SINGLE_QUOTE);

  std::string IRIREF(std::string_view p, std::string_view v);
  FRIEND_TEST(WriterGrammarNT, RULE_8_IRIREF);
  FRIEND_TEST(WriterGrammarTTL, RULE_18_IRIREF);

  std::string PrefixedNameUnsafe(std::string_view p, std::string_view v);
  std::string PrefixedName(std::string_view p, std::string_view v);
  FRIEND_TEST(WriterGrammarTTL, RULE_136s_PREFIXEDNAME);

  std::string encodeIRIREF(std::string_view s);
  FRIEND_TEST(WriterGrammarNT, RULE_8_IRIREF_CONVERT);
  FRIEND_TEST(WriterGrammarTTL, RULE_18_IRIREF_CONVERT);

  std::string encodePERCENT(char c);
  std::string encodePERCENT(std::string_view s);
  std::string encodePERCENT(uint32_t codepoint);
  FRIEND_TEST(WriterGrammarTTL, RULE_170s_PERCENT_CODEPOINT);
  FRIEND_TEST(WriterGrammarTTL, RULE_170s_PERCENT_ASCII);
  FRIEND_TEST(WriterGrammarTTL, RULE_170s_PERCENT_UTF8);

  std::string encodePN_LOCAL(std::string_view s);
  FRIEND_TEST(WriterGrammarTTL, RULE_168s_PN_LOCAL);

  std::string encodePN_PREFIX(std::string_view s);
  FRIEND_TEST(WriterGrammarTTL, RULE_167s_PN_PREFIX);

  uint8_t utf8Length(char c);
  uint8_t utf8Length(std::string_view s);
  FRIEND_TEST(WriterGrammar, UTF8_LENGTH_ASCII);
  FRIEND_TEST(WriterGrammar, UTF8_LENGTH_UTF8);

  uint32_t utf8Codepoint(std::string_view s);
  FRIEND_TEST(WriterGrammar, UTF8_CODEPOINT_ASCII);
  FRIEND_TEST(WriterGrammar, UTF8_CODEPOINT_UTF8);

  std::string UCHAR(char c);
  std::string UCHAR(std::string_view s);
  std::string UCHAR(uint32_t codepoint);
  FRIEND_TEST(WriterGrammarNT, RULE_10_UCHAR_CODEPOINT);
  FRIEND_TEST(WriterGrammarNT, RULE_10_UCHAR_ASCII);
  FRIEND_TEST(WriterGrammarNT, RULE_10_UCHAR_UTF8);
  FRIEND_TEST(WriterGrammarTTL, RULE_26_UCHAR_CODEPOINT);
  FRIEND_TEST(WriterGrammarTTL, RULE_26_UCHAR_ASCII);
  FRIEND_TEST(WriterGrammarTTL, RULE_26_UCHAR_UTF8);

 protected:
  // Config
  const osm2rdf::config::Config _config;

  // Prefix
  std::unordered_map<std::string, std::string> _prefixes;

  // Output
  osm2rdf::util::Output* _out;

  // Counter
  uint64_t* _headerLines;
  uint64_t* _lineCount;
  uint64_t _blankNodeCounter = 0;
  // Number of parts.
  std::size_t _numOuts;

};
}  // namespace osm2rdf::ttl

#endif  // OSM2RDF_TTL_WRITER_H_
