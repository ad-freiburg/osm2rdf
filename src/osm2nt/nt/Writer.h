// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_WRITER_H_
#define OSM2NT_NT_WRITER_H_

#include <fstream>
#include <ostream>
#include <string>
#include <unordered_map>

#include "osmium/geom/wkt.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/config/Config.h"

#include "osm2nt/nt/BlankNode.h"
#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/Literal.h"

#include "osm2nt/osm/SimplifyingWKTFactory.h"

namespace osm2nt {
namespace nt {

class Writer {
 public:
  explicit Writer(const osm2nt::config::Config& config);
  void writeHeader() const;

  template<typename S, typename O>
  void writeTriple(const S& s, const osm2nt::nt::IRI& p, const O& o);

  // OSM
  void writeOsmArea(const osmium::Area& area);
  template<typename S>
  void writeOsmBox(const S& s, const osm2nt::nt::IRI& p,
                   const osmium::Box& box);
  template<typename S>
  void writeOsmLocation(const S& s,
                        const osmium::Location& l);
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

  // Element
  void write(const osm2nt::nt::BlankNode& b);
  void write(const osm2nt::nt::IRI& i);
  void write(const osm2nt::nt::LangTag& l);
  void write(const osm2nt::nt::Literal& l);

  // Config
  osm2nt::config::Config _config;
  std::unordered_map<std::string, std::string> _prefixes;
  // Output
  std::ostream* _out;
  std::ofstream _outFile;
  // Factories
  osmium::geom::WKTFactory<> _wktFactory;
  osm2nt::osm::SimplifyingWKTFactory<> _simplifyingWktFactory;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_WRITER_H_
