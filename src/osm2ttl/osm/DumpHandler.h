// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_DUMPHANDLER_H_
#define OSM2TTL_OSM_DUMPHANDLER_H_

#include <ostream>

#include "osmium/handler.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/AreaHandler.h"
#include "osm2ttl/ttl/Writer.h"

namespace osm2ttl {
namespace osm {

class DumpHandler : public osmium::handler::Handler {
 public:
  DumpHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer* writer,
              osm2ttl::osm::AreaHandler* areaHandler);
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
  const osmium::memory::item_size_type EMPTY_TAG_SIZE = 8;
 protected:
  const osm2ttl::config::Config _config;
  osm2ttl::util::DispatchQueue _queue;
  osm2ttl::ttl::Writer* _writer;
  osm2ttl::osm::AreaHandler* _areaHandler;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_DUMPHANDLER_H_
