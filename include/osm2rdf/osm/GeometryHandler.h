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

#ifndef OSM2RDF_OSM_GEOMETRYHANDLER_H_
#define OSM2RDF_OSM_GEOMETRYHANDLER_H_

#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "boost/archive/binary_oarchive.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/version.hpp"
#include "gtest/gtest_prod.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/geometry/Node.h"
#include "osm2rdf/geometry/Way.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/CacheFile.h"
#include "osm2rdf/util/DirectedGraph.h"
#include "osm2rdf/util/Output.h"

namespace osm2rdf::osm {

template <typename W>
class GeometryHandler {
 public:
  GeometryHandler(const osm2rdf::config::Config& config,
                  osm2rdf::ttl::Writer<W>* writer);
  ~GeometryHandler();

  // Add data
  void area(const osm2rdf::osm::Area& area);
  void node(const osm2rdf::osm::Node& node);
  void relation(const osm2rdf::osm::Relation& relation);
  void way(const osm2rdf::osm::Way& way);

  // Calculate data
  void calculateRelations();

  // Global config
  osm2rdf::config::Config _config;
  osm2rdf::ttl::Writer<W>* _writer;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_GEOMETRYHANDLER_H_
