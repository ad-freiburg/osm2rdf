// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_WRITER_H_
#define OSM2TTL_TTL_WRITER_H_

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
static const int k0xC0 = 0xC0;
static const int k0xD6 = 0xD6;
static const int k0xD8 = 0xD8;
static const int k0xE0 = 0xE0;
static const int k0xF0 = 0xF0;
static const int k0xF6 = 0xF6;
static const int k0xF8 = 0xF8;
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
static const int k0x10000 = 0x10000;
static const int k0xEFFFF = 0xEFFFF;

static const int UTF8_CODEPOINT_OFFSET_BYTE4 = 18;
static const int UTF8_CODEPOINT_OFFSET_BYTE3 = 12;
static const int UTF8_CODEPOINT_OFFSET_BYTE2 = 6;
static const int NUM_BITS_IN_BYTE = 8;
static const int MASK_BITS_OF_ONE_BYTE = 0xFF;
static const int NUM_BITS_IN_NIBBLE = 4;

#include <fstream>
#include <ostream>
#include <string>

#include "boost/iostreams/filtering_stream.hpp"
#include "osm2ttl/ttl/Constants.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/Tag.h"
#include "osm2ttl/osm/TagList.h"
#include "osm2ttl/osm/Way.h"

namespace osm2ttl {
namespace ttl {

template<typename T>
class Writer {
 public:
  explicit Writer(const osm2ttl::config::Config& config);
  ~Writer();
  bool open();
  void close();

  void writeHeader();

  void writeTriple(const std::string& s, const std::string& p, const std::string& o);

  // addPrefix adds the given prefix and value. If the prefix already exists false is returned.
  bool addPrefix(std::string_view p, std::string_view v);
  // resolvePrefix resolves the given prefix. If the prefix is unknown it is returned as provided.
  std::string resolvePrefix(std::string_view p);

  // generateBlankNode creates a new unique identifier for a blank node.
  std::string generateBlankNode();
  // generateIRI creates a IRI from given prefix p and ID value v.
  std::string generateIRI(std::string_view p, uint64_t v);
  // generateIRI creates a IRI from given prefix p and string value v.
  std::string generateIRI(std::string_view p, std::string_view v);
  // generateLangTag creates a LangTag from the given string.
  std::string generateLangTag(std::string_view s);
  // generateLangTag creates a Literal from the given string value v.
  // If suffix s is not empty, it will be appended as is.
  std::string generateLiteral(std::string_view v, std::string_view s);

  std::string encodePN_LOCAL(std::string_view s);
  uint32_t utf8Codepoint(std::string_view s);
 protected:
  std::string formatIRI(std::string_view p, std::string_view v);
  std::string STRING_LITERAL_QUOTE(std::string_view s);
  std::string IRIREF(std::string_view p, std::string_view v);
  std::string PrefixedName(std::string_view p, std::string_view v);
  uint8_t utf8Length(char c);
  uint8_t utf8Length(std::string_view s);
  std::string UCHAR(char c);
  std::string encodeIRIREF(std::string_view s);
  std::string encodePERCENT(std::string_view s);

  // Config
  uint64_t _blankNodeCounter = 0;
  const osm2ttl::config::Config _config;

  // Prefix
  std::unordered_map<std::string, std::string> _prefixes;

  // Output
  boost::iostreams::filtering_ostream _out;
  std::ofstream _outFile;
};
}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
