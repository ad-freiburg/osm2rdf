// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_WRITER_H_
#define OSM2NT_NT_WRITER_H_

#include <fstream>
#include <ostream>
#include <string>

#include "osmium/geom/wkt.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/config/Config.h"
#include "osm2nt/nt/Triple.h"
#include "osm2nt/osm/SimplifyingWKTFactory.h"

namespace osm2nt {
namespace nt {

class Writer {
 public:
  explicit Writer(const osm2nt::config::Config& config);
  static bool tagKeyEndsWith(const osmium::Tag& tag, const std::string& needle);
  void writeTriple(const osm2nt::nt::Triple& t);
  void writeOsmArea(const osmium::Area& area);
  void writeOsmBox(const osm2nt::nt::Subject* s,
                   const osm2nt::nt::Predicate* p, const osmium::Box& box);
  void writeOsmLocation(const osm2nt::nt::Subject* s,
                        const osmium::Location& l);
  void writeOsmNode(const osmium::Node& node);
  void writeOsmRelation(const osmium::Relation& relation);
  void writeOsmRelationMembers(const osm2nt::nt::Subject* s,
                               const osmium::RelationMemberList& members);
  void writeOsmTag(const osm2nt::nt::Subject* s, const osmium::Tag& tag);
  void writeOsmTagList(const osm2nt::nt::Subject* s,
                       const osmium::TagList& tags);
  void writeOsmWay(const osmium::Way& way);
  void writeOsmWayNodeList(const osm2nt::nt::Subject* s,
                           const osmium::WayNodeList& nodes);

 protected:
  osm2nt::config::Config config;
  std::ostream* out;
  std::ofstream outFile;
  osmium::geom::WKTFactory<> wktFactory;
  osm2nt::osm::SimplifyingWKTFactory<> simplifyingWktFactory;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_WRITER_H_
