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
static const int BIT_IN_BYTE = 8;
static const int BITS_OF_BYTE = 0xFF;
static const int BIT_IN_NIBBLE = 4;

#include <fstream>
#include <mutex>
#include <ostream>
#include <string>

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
#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"

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

  static std::string generateBlankNode();
  static std::string generateIRI(std::string_view p, uint64_t v);
  static std::string generateIRI(std::string_view p, std::string_view v);
  static std::string generateLangTag(std::string_view s);
  static std::string generateLiteral(std::string_view v, std::string_view s);

  // OSM2TTL
  void write(const osm2ttl::osm::Area& area);
  void write(const osm2ttl::osm::Node& node);
  void write(const osmium::Node& node);
  void write(const osm2ttl::osm::Relation& relation);
  void write(const osmium::Relation& relation);
  void write(const osm2ttl::osm::Way& way);
  template<typename G>
  void writeBoostGeometry(const std::string& s, const std::string& p,
                          const G& g);

  void writeBox(const std::string& s, const std::string& p,
                const osm2ttl::geometry::Box& box);
  void writeBox(const std::string& s, const std::string& p,
                const osm2ttl::osm::Box& box);
  void writeTag(const std::string& s, const osm2ttl::osm::Tag& tag);
  void writeTag(const std::string& s, const osmium::Tag& tag);
  void writeTagList(const std::string& s, const osm2ttl::osm::TagList& tags);
  void writeTagList(const std::string& s, const osmium::TagList& tags);

  static std::string encodePN_LOCAL(std::string_view s);
  static uint32_t utf8Codepoint(std::string_view s);
 protected:
  static std::string formatIRI(std::string_view p, std::string_view v);
  static std::string STRING_LITERAL_QUOTE(std::string_view s);
  static std::string IRIREF(std::string_view p, std::string_view v);
  static std::string PrefixedName(std::string_view p, std::string_view v);
  static uint8_t utf8Length(char c);
  static uint8_t utf8Length(std::string_view s);
  static std::string UCHAR(char c);
  static std::string encodeIRIREF(std::string_view s);
  static std::string encodePERCENT(std::string_view s);

  // Config
  static uint64_t _blankNodeCounter;
  const osm2ttl::config::Config _config;

  // Prefix
  const static inline std::unordered_map<std::string, std::string> _prefixes{
// well-known prefixes
      {osm2ttl::ttl::constants::NAMESPACE__GEOSPARQL, "http://www.opengis.net/ont/geosparql#"},
      {osm2ttl::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, "http://www.wikidata.org/entity/"},
      {osm2ttl::ttl::constants::NAMESPACE__XML_SCHEMA, "http://www.w3.org/2001/XMLSchema#"},
      {osm2ttl::ttl::constants::NAMESPACE__RDF, "http://www.w3.org/1999/02/22-rdf-syntax-ns#"},
      {osm2ttl::ttl::constants::NAMESPACE__OPENGIS, "http://www.opengis.net/rdf#"},
// osm prefixes
      {osm2ttl::ttl::constants::NAMESPACE__OSM, "https://www.openstreetmap.org/"},
// https://wiki.openstreetmap.org/wiki/Sophox#How_OSM_data_is_stored
// https://github.com/Sophox/sophox/blob/master/osm2rdf/osmutils.py#L35-L39
      {osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, "https://www.openstreetmap.org/node/"},
      {osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, "https://www.openstreetmap.org/relation/"},
      {osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, "https://www.openstreetmap.org/wiki/Key:"},
      {osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, "https://www.openstreetmap.org/way/"},
      {osm2ttl::ttl::constants::NAMESPACE__OSM_META, "https://www.openstreetmap.org/meta/"}
  };

  // Output
  std::ostream* _out;
  std::ofstream _outFile;
  std::mutex _outMutex;
};
}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
