// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_DUMPHANDLER_H_
#define OSM2TTL_OSM_DUMPHANDLER_H_

#include <ostream>

#include "osm2ttl/config/Config.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/handler.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

namespace osm2ttl {
namespace osm {

template <typename W>
class DumpHandler : public osmium::handler::Handler {
 public:
  DumpHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer<W>* writer);
  void area(const osm2ttl::osm::Area& area);
  void node(const osm2ttl::osm::Node& node);
  void relation(const osm2ttl::osm::Relation& relation);
  void way(const osm2ttl::osm::Way& way);

 protected:
  template <typename T>
  void write(const T& o);
  const osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_DUMPHANDLER_H_
