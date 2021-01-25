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

namespace osm2ttl::osm {

template <typename W>
class OsmiumHandler : public osmium::handler::Handler {
 public:
  OsmiumHandler(const osm2ttl::config::Config& config,
                osm2ttl::ttl::Writer<W>* writer);
  void handle();
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);

 protected:
  osm2ttl::config::Config _config;
  osm2ttl::osm::DumpHandler<W> _dumpHandler;
  osm2ttl::osm::GeometryHandler<W> _geometryHandler;
  size_t _areasSeen = 0;
  size_t _areasDumped = 0;
  size_t _areaGeometriesHandled = 0;
  size_t _nodesSeen = 0;
  size_t _nodesDumped = 0;
  size_t _nodeGeometriesHandled = 0;
  size_t _relationsSeen = 0;
  size_t _relationsDumped = 0;
  size_t _relationGeometriesHandled = 0;
  size_t _waysSeen = 0;
  size_t _waysDumped = 0;
  size_t _wayGeometriesHandled = 0;
};
}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_OSMIUMHANDLER_H
