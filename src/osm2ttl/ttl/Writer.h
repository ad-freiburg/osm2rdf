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

  // OSM2TTL
  void writeArea(const osm2ttl::osm::Area& area);
  template<typename S, typename G>
  void writeBoostGeometry(const S& s, const osm2ttl::ttl::IRI& p,
                          const G& g);
  template<typename S>
  void writeBox(const S& s, const osm2ttl::ttl::IRI& p,
                const osm2ttl::osm::Box& box);
  void writeNode(const osm2ttl::osm::Node& node);
  void writeRelation(const osm2ttl::osm::Relation& relation);
  template<typename S>
  void writeTag(const S& s, const osm2ttl::osm::Tag& tag);
  template<typename S>
  void writeTagList(const S& s, const osm2ttl::osm::TagList& tags);
  void writeWay(const osm2ttl::osm::Way& way);

 protected:
  // Helper
  static bool contains(std::string_view s, std::string_view n);
  static bool endsWith(std::string_view s, std::string_view n);
  static bool startsWith(std::string_view s, std::string_view n);

  // Config
  const osm2ttl::config::Config _config;

  // Queues
  osm2ttl::util::DispatchQueue _outQueue;
  osm2ttl::util::DispatchQueue _convertQueue;

  // Output
  std::ostream* _out;
  std::ofstream _outFile;
  std::mutex _outMutex;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_WRITER_H_
