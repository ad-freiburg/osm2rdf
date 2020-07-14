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
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/geometry/Location.h"

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Tag.h"
#include "osm2ttl/osm/TagList.h"
#include "osm2ttl/osm/WKTFactory.h"

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/Literal.h"

#include "osm2ttl/util/DispatchQueue.h"

namespace osm2ttl {
namespace ttl {

class Writer {
 public:
  explicit Writer(const osm2ttl::config::Config& config);
  ~Writer();
  bool open();
  void close();

  void writeHeader();

  template<typename S, typename O>
  void writeTriple(const S& s, const osm2ttl::ttl::IRI& p, const O& o);

  // Osmium
  template<typename S>
  void writeOsmiumBox(const S& s, const osm2ttl::ttl::IRI& p,
                   const osmium::Box& box);
  void writeOsmiumRelation(const osmium::Relation& relation);
  template<typename S>
  void writeOsmiumRelationMembers(const S& s,
                               const osmium::RelationMemberList& members);
  template<typename S>
  void writeOsmiumTag(const S& s, const osmium::Tag& tag);
  template<typename S>
  void writeOsmiumTagList(const S& s,
                       const osmium::TagList& tags);
  void writeOsmiumWay(const osmium::Way& way);
  template<typename S>
  void writeOsmiumWayNodeList(const S& s,
                           const osmium::WayNodeList& nodes);
  // OSM2TTL
  void writeArea(const osm2ttl::osm::Area& area);
  template<typename S, typename G>
  void writeBoostGeometry(const S& s, const osm2ttl::ttl::IRI& p,
                          const G& g);
  template<typename S>
  void writeBox(const S& s, const osm2ttl::ttl::IRI& p,
                const osm2ttl::osm::Box& box);
  void writeNode(const osm2ttl::osm::Node& node);
  template<typename S>
  void writeTag(const S& s, const osm2ttl::osm::Tag& tag);
  template<typename S>
  void writeTagList(const S& s, const osm2ttl::osm::TagList& tags);

 protected:
  // Helper
  static bool contains(std::string_view s, std::string_view n);
  static bool endsWith(std::string_view s, std::string_view n);
  static bool startsWith(std::string_view s, std::string_view n);

  // Config
  const osm2ttl::config::Config _config;
  osm2ttl::util::DispatchQueue _queue;

  // Output
  std::ostream* _out;
  std::ofstream _outFile;
  std::mutex _outMutex;

  // Helper
  osm2ttl::osm::WKTFactory* _factory;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
