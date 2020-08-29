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

#include <fstream>
#include <mutex>
#include <ostream>
#include <string>

#include "osm2ttl/config/Config.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/Tag.h"
#include "osm2ttl/osm/TagList.h"
#include "osm2ttl/osm/Way.h"
#include "osm2ttl/util/DispatchQueue.h"
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

  void writeTriple(const std::string& s, const std::string& p,
                   const std::string& o);

  static std::string generateBlankNode();
  static std::string generateIRI(const std::string& p, uint64_t v);
  static std::string generateIRI(const std::string& p, const std::string& v);
  static std::string generateLangTag(const std::string& s);
  static std::string generateLiteral(const std::string& v, const std::string& s);

  // OSM2TTL
  void write(const osm2ttl::osm::Area& area);
  void write(const osm2ttl::osm::Node& node);
  void write(const osm2ttl::osm::Relation& relation);
  void write(const osm2ttl::osm::Way& way);
  template<typename G>
  void writeBoostGeometry(const std::string& s, const std::string& p,
                          const G& g);

  void writeBox(const std::string& s, const std::string& p,
                const osm2ttl::osm::Box& box);
  void writeTag(const std::string& s, const osm2ttl::osm::Tag& tag);
  void writeTagList(const std::string& s, const osm2ttl::osm::TagList& tags);

  static std::string encodePN_LOCAL(const std::string& s);
  static uint32_t utf8Codepoint(const std::string& s);
 protected:
  static std::string formatIRI(const std::string& p, const std::string& v);
  static std::string STRING_LITERAL_QUOTE(const std::string& s);
  static std::string IRIREF(const std::string& p, const std::string& v);
  static std::string PrefixedName(const std::string& p, const std::string& v);
  static uint8_t utf8Length(char c);
  static uint8_t utf8Length(const std::string& s);
  static std::string UCHAR(char c);
  static std::string encodeIRIREF(const std::string& s);
  static std::string encodePERCENT(const std::string& s);

  std::string _kGeoHasGeometry;
  std::string _kGeoWktLiteral;
  std::string _kOsmEnvelope;
  std::string _kOsmNode;
  std::string _kOsmRelation;
  std::string _kOsmWay;
  std::string _kOsmWikipedia;
  std::string _kOsmmPos;
  std::string _kOsmwayIsClosed;
  std::string _kOsmwayNode;
  std::string _kOsmwayNodeCount;
  std::string _kOsmwayUniqueNodeCount;
  std::string _kRdfType;
  std::string _kXsdInteger;

  std::string _kLiteralNo;
  std::string _kLiteralYes;

  // Config
  static uint64_t _blankNodeCounter;
  const osm2ttl::config::Config _config;

  // Queues
  osm2ttl::util::DispatchQueue _convertGeometryQueue;

  // Prefix
  const static inline std::unordered_map<std::string, std::string> _prefixes{
// well-known prefixes
      {"geo", "http://www.opengis.net/ont/geosparql#"},
      {"wd", "http://www.wikidata.org/entity/"},
      {"xsd", "http://www.w3.org/2001/XMLSchema#"},
      {"rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#"},
      {"rdfs", "\"http://www.w3.org/2000/01/rdf-schema#"},
// osm prefixes
// https://wiki.openstreetmap.org/wiki/Sophox#How_OSM_data_is_stored
// https://github.com/Sophox/sophox/blob/master/osm2rdf/osmutils.py#L35-L39
      {"osmnode", "https://www.openstreetmap.org/node/"},
      {"osmrel", "https://www.openstreetmap.org/relation/"},
      {"osmt", "https://www.openstreetmap.org/wiki/Key:"},
      {"osmway", "https://www.openstreetmap.org/way/"},
      {"osmm", "https://www.openstreetmap.org/meta/"},
// own prefixes
      {"osm", "https://www.openstreetmap.org/"},
      {"osma", "https://www.openstreetmap.org/area/"},
      {"osml", "https://www.openstreetmap.org/location/"}
  };

  // Output
  std::ostream* _out;
  std::ofstream _outFile;
  std::mutex _outMutex;
};
}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
