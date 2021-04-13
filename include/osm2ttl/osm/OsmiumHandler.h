// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2TTL_OSM_OSMIUMHANDLER_H
#define OSM2TTL_OSM_OSMIUMHANDLER_H

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/FactHandler.h"
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

  [[nodiscard]] size_t areasSeen() const;
  [[nodiscard]] size_t areasDumped() const;
  [[nodiscard]] size_t areaGeometriesHandled() const;
  [[nodiscard]] size_t nodesSeen() const;
  [[nodiscard]] size_t nodesDumped() const;
  [[nodiscard]] size_t nodeGeometriesHandled() const;
  [[nodiscard]] size_t relationsSeen() const;
  [[nodiscard]] size_t relationsDumped() const;
  [[nodiscard]] size_t relationGeometriesHandled() const;
  [[nodiscard]] size_t waysSeen() const;
  [[nodiscard]] size_t waysDumped() const;
  [[nodiscard]] size_t wayGeometriesHandled() const;

 protected:
  osm2ttl::config::Config _config;
  osm2ttl::osm::FactHandler<W> _dumpHandler;
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
