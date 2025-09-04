// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2RDF_OSM_OSMIUMHANDLER_H
#define OSM2RDF_OSM_OSMIUMHANDLER_H

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/GeometryHandler.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/ProgressBar.h"
#include "osmium/handler.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/area/assembler.hpp"
#include "osmium/osm/way.hpp"

namespace osm2rdf::osm {

template <typename W>
class OsmiumHandler : public osmium::handler::Handler {
 public:
  OsmiumHandler(const osm2rdf::config::Config& config,
                osm2rdf::osm::FactHandler<W>* factHandler,
                osm2rdf::osm::GeometryHandler<W>* geomHandler);
  void handle();
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(osmium::Way& way);

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
  osm2rdf::config::Config _config;
  osm2rdf::osm::FactHandler<W>* _factHandler;
  osm2rdf::osm::GeometryHandler<W>* _geometryHandler;
  osm2rdf::osm::LocationHandler* _locationHandler;

  osm2rdf::osm::RelationHandler _relationHandler;
  osm2rdf::util::ProgressBar _progressBar;
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

  size_t _numTasksDone = 0;
 private:
  void handleBuffers(std::vector<osmium::memory::Buffer>& buffers, size_t len, std::vector<osmium::memory::Buffer>& areaBuffers, size_t& ai, osmium::area::MultipolygonManager<osmium::area::Assembler>& mp_manager);
  void handleAreaBuffers(std::vector<osmium::memory::Buffer>& areaBuffers, size_t len);
};
}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_OSMIUMHANDLER_H
