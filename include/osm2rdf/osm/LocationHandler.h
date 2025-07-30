// Copyright 2020, University of Freiburg
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

#ifndef OSM2RDF_OSM_LOCATIONHANDLER_H_
#define OSM2RDF_OSM_LOCATIONHANDLER_H_

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/DenseMemIndex.h"
#include "osm2rdf/util/CacheFile.h"
#include "osmium/handler.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/dense_file_array.hpp"
#include "osmium/index/map/flex_mem.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/types.hpp"
#include "osmium/osm/way.hpp"

namespace osm2rdf::osm {

// Partially based on osmium::handler::ObjectRelations
class LocationHandler : public osmium::handler::Handler {
 public:
  virtual ~LocationHandler() {}
  virtual void node(const osmium::Node& node) = 0;
  virtual void way(osmium::Way& way) = 0;
  [[nodiscard]] virtual osmium::Location get_node_location(
      const osmium::object_id_type id) const = 0;
  [[nodiscard]] virtual bool get_node_is_tagged(
      const osmium::object_id_type id) const = 0;
  // Helper creating the correct instance.
  static LocationHandler* create(const osm2rdf::config::Config& config,
                                 size_t nodeIdMin, size_t nodeIdMax);
};

template <typename T>
class LocationHandlerImpl : public LocationHandler {
 public:
  explicit LocationHandlerImpl(const osm2rdf::config::Config& config,
                               size_t nodeIdMin, size_t nodeIdMax);
  void node(const osmium::Node& node);
  void way(osmium::Way& way);
  [[nodiscard]] osmium::Location get_node_location(
      const osmium::object_id_type nodeId) const;
  [[nodiscard]] bool get_node_is_tagged(
      const osmium::object_id_type nodeId) const;

 protected:
  T _index;
  osmium::handler::NodeLocationsForWays<T> _handler;
};

template <>
class LocationHandlerImpl<osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::Location>>
    : public LocationHandler {
 public:
  explicit LocationHandlerImpl(const osm2rdf::config::Config& config,
                               size_t nodeIdMin, size_t nodeIdMax);
  void node(const osmium::Node& node);
  void way(osmium::Way& way);
  [[nodiscard]] osmium::Location get_node_location(
      const osmium::object_id_type nodeId) const;
  [[nodiscard]] bool get_node_is_tagged(
      const osmium::object_id_type nodeId) const;

 protected:
  osm2rdf::util::CacheFile _cacheFile;
  osmium::index::map::SparseFileArray<osmium::unsigned_object_id_type,
                                      osmium::Location>
      _index;
  osmium::handler::NodeLocationsForWays<osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>
      _handler;
};

template <>
class LocationHandlerImpl<osmium::index::map::DenseFileArray<
    osmium::unsigned_object_id_type, osmium::Location>>
    : public LocationHandler {
 public:
  explicit LocationHandlerImpl(const osm2rdf::config::Config& config,
                               size_t nodeIdMin, size_t nodeIdMax);
  void node(const osmium::Node& node);
  void way(osmium::Way& way);
  [[nodiscard]] osmium::Location get_node_location(
      const osmium::object_id_type nodeId) const;
  [[nodiscard]] bool get_node_is_tagged(
      const osmium::object_id_type nodeId) const;

 protected:
  osm2rdf::util::CacheFile _cacheFile;
  osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type,
                                     osmium::Location>
      _index;
  osmium::handler::NodeLocationsForWays<osmium::index::map::DenseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>
      _handler;
};

template <>
class LocationHandlerImpl<osm2rdf::osm::DenseMemIndex<
    osmium::unsigned_object_id_type, osmium::Location>>
    : public LocationHandler {
 public:
  explicit LocationHandlerImpl(const osm2rdf::config::Config& config,
                               size_t nodeIdMin, size_t nodeIdMax);
  void node(const osmium::Node& node);
  void way(osmium::Way& way);
  [[nodiscard]] osmium::Location get_node_location(
      const osmium::object_id_type nodeId) const;
  [[nodiscard]] bool get_node_is_tagged(
      const osmium::object_id_type nodeId) const;

 protected:
  osm2rdf::osm::DenseMemIndex<osmium::unsigned_object_id_type, osmium::Location>
      _index;
  osmium::handler::NodeLocationsForWays<osm2rdf::osm::DenseMemIndex<
      osmium::unsigned_object_id_type, osmium::Location>>
      _handler;
};

using LocationHandlerRAMDense = LocationHandlerImpl<osm2rdf::osm::DenseMemIndex<
    osmium::unsigned_object_id_type, osmium::Location>>;
using LocationHandlerRAMFlex = LocationHandlerImpl<osmium::index::map::FlexMem<
    osmium::unsigned_object_id_type, osmium::Location>>;
using LocationHandlerFSSparse =
    LocationHandlerImpl<osmium::index::map::SparseFileArray<
        osmium::unsigned_object_id_type, osmium::Location>>;
using LocationHandlerFSDense =
    LocationHandlerImpl<osmium::index::map::DenseFileArray<
        osmium::unsigned_object_id_type, osmium::Location>>;

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_LOCATIONHANDLER_H_
