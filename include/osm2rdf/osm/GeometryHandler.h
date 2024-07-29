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

#include "gtest/gtest_prod.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/CacheFile.h"
#include "osm2rdf/util/DirectedGraph.h"
#include "osm2rdf/util/Output.h"
#include "osm2rdf/util/ProgressBar.h"
#include "spatialjoin/Sweeper.h"
#include "spatialjoin/WKTParse.h"
#include "util/geo/Geo.h"

namespace osm2rdf::osm {

enum class AreaFromType { RELATION, WAY };

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

 private:
  sj::Sweeper _sweeper;
  std::vector<sj::WriteBatch> _parseBatches;

  std::string areaNS(AreaFromType type) const;

  static ::util::geo::I32Point transform(const ::util::geo::Point<double>& loc);

  static ::util::geo::I32Box transform(const ::util::geo::Box<double>& box);

  static ::util::geo::I32Line transform(const ::util::geo::DLine& way);
  static ::util::geo::I32MultiPolygon transform(
      const ::util::geo::DMultiPolygon& area);

  void writeRelCb(size_t t, const std::string& a, const std::string& b,
                  const std::string& pred);
  void progressCb(size_t progr);

  osm2rdf::util::ProgressBar _progressBar;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_GEOMETRYHANDLER_H_
