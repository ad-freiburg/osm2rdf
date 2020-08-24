// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_WRITER_H_
#define OSM2TTL_TTL_WRITER_H_

#include <fstream>
#include <ostream>
#include <string>
#include <mutex>

#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"

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

 protected:
  static std::string formatIRI(const std::string& p, const std::string& v);
  static std::string STRING_LITERAL_QUOTE(const std::string& s);
  static std::string IRIREF(const std::string& p, const std::string& v);
  static std::string PrefixedName(const std::string& p, const std::string& v);
  static std::string ECHAR(char c);
  static uint8_t utf8Length(char c);
  static uint8_t utf8Length(const std::string& s);
  static uint32_t utf8Codepoint(const std::string& s);
  static std::string UCHAR(char c);
  static std::string encodeIRIREF(const std::string& s);
  static std::string encodePN_LOCAL(const std::string& s);
  static std::string encodePERCENT(const std::string& s);

  // Config
  static uint64_t _blankNodeCounter;
  const osm2ttl::config::Config _config;

  // Queues
  osm2ttl::util::DispatchQueue _convertStringQueue;
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

template< typename T> uint64_t osm2ttl::ttl::Writer<T>::_blankNodeCounter;
template class osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT>;
template class osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL>;
template class osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER>;
}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
