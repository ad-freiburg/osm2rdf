// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_WRITER_H_
#define OSM2TTL_TTL_WRITER_H_

#include <fstream>
#include <ostream>
#include <string>
#include <unordered_set>

#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/osm/WKTFactory.h"

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/Literal.h"

namespace osm2ttl {
namespace ttl {

class Writer {
 public:
  explicit Writer(const osm2ttl::config::Config& config);
  ~Writer();
  bool open();
  void close();

  void writeHeader() const;

  template<typename S, typename O>
  void writeTriple(const S& s, const osm2ttl::ttl::IRI& p, const O& o);

  // OSM
  void writeOsmArea(const osmium::Area& area);
  template<typename S>
  void writeOsmBox(const S& s, const osm2ttl::ttl::IRI& p,
                   const osmium::Box& box);
  template<typename S>
  void writeOsmLocation(const S& s,
                        const osmium::Location& location);
  void writeOsmNode(const osmium::Node& node);
  void writeOsmRelation(const osmium::Relation& relation);
  template<typename S>
  void writeOsmRelationMembers(const S& s,
                               const osmium::RelationMemberList& members);
  template<typename S>
  void writeOsmTag(const S& s, const osmium::Tag& tag);
  template<typename S>
  void writeOsmTagList(const S& s,
                       const osmium::TagList& tags);
  void writeOsmWay(const osmium::Way& way);
  template<typename S>
  void writeOsmWayNodeList(const S& s,
                           const osmium::WayNodeList& nodes);

 protected:
  // Helper
  static bool contains(std::string_view s, std::string_view n);
  static bool endsWith(std::string_view s, std::string_view n);
  static bool startsWith(std::string_view s, std::string_view n);
  static std::string urlencode(std::string_view s);
  // Used for PN_LOCAL_ESC
  // https://www.w3.org/TR/turtle/#grammar-production-PN_LOCAL_ESC
  static std::string urlescape(std::string_view s);

  // Element
  void write(const osm2ttl::ttl::BlankNode& b);
  void write(const osm2ttl::ttl::IRI& i);
  void write(const osm2ttl::ttl::LangTag& l);
  void write(const osm2ttl::ttl::Literal& l);

  // Config
  osm2ttl::config::Config _config;

  // Output
  std::ostream* _out;
  std::ofstream _outFile;

  // Helper
  std::unordered_set<std::string> _basicDataTagKeys;
  osm2ttl::osm::WKTFactory* _factory;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
