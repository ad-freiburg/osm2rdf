// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_OSMIUMHANDLER_H
#define OSM2TTL_OSM_OSMIUMHANDLER_H

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/DumpHandler.h"
#include "osm2ttl/osm/GeometryHandler.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/handler.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

namespace osm2ttl {
namespace osm {

template <typename W>
class OsmiumHandler : public osmium::handler::Handler {
 public:
  OsmiumHandler(const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer);
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
  void calculateRelations();

 protected:
  osm2ttl::config::Config _config;
  osm2ttl::osm::DumpHandler<W> _dumpHandler;
  osm2ttl::osm::GeometryHandler<W> _geometryHandler;
};
}
}

#endif  // OSM2TTL_OSM_OSMIUMHANDLER_H
