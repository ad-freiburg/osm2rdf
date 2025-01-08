// Copyright 2024, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

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

#ifndef OSM2RDF_OSM_COUNTHANDLER_H
#define OSM2RDF_OSM_COUNTHANDLER_H

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/LocationHandler.h"

namespace osm2rdf::osm {

class CountHandler : public osmium::handler::Handler {
 public:
  CountHandler(const osm2rdf::config::Config& config) : _config(config) {};
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
  void prepare_for_lookup();

  size_t numNodes() const;
  size_t numRelations() const;
  size_t numWays() const;

  size_t minNodeId() const { return _minId; };
  size_t maxNodeId() const { return _maxId; };

 protected:
  size_t _numNodes = 0;
  size_t _numRelations = 0;
  size_t _numWays = 0;
  bool _firstPassDone = false;
  size_t _minId = std::numeric_limits<size_t>::max();
  size_t _maxId = 0;

  osm2rdf::config::Config _config;
};
}

#endif  // OSM2RDF_OSM_RELATIONHANDLER_H
