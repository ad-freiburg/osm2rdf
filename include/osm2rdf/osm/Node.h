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

#ifndef OSM2RDF_OSM_NODE_H_
#define OSM2RDF_OSM_NODE_H_

#include "osm2rdf/osm/TagList.h"
#include "osm2rdf/osm/Generic.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"
#include "util/geo/Geo.h"

namespace osm2rdf::osm {

class Node {
 public:
  typedef uint64_t id_t;
  explicit Node();
  explicit Node(const osmium::Node& node);
  explicit Node(const osmium::NodeRef& nodeRef);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] osm2rdf::osm::generic::changeset_id_t changeset() const noexcept;
  [[nodiscard]] std::time_t timestamp() const noexcept;
  [[nodiscard]] std::string user() const noexcept;
  [[nodiscard]] id_t uid() const noexcept;
  [[nodiscard]] osm2rdf::osm::generic::version_t version() const noexcept;
  [[nodiscard]] bool visible() const noexcept;
  [[nodiscard]] const ::util::geo::DBox envelope() const noexcept;
  [[nodiscard]] const ::util::geo::DPoint& geom() const noexcept;
  [[nodiscard]] const osm2rdf::osm::TagList& tags() const noexcept;

  bool operator==(const osm2rdf::osm::Node& other) const noexcept;
  bool operator!=(const osm2rdf::osm::Node& other) const noexcept;

 protected:
  id_t _id;
  osm2rdf::osm::generic::changeset_id_t _changeset;
  std::time_t _timestamp;
  std::string _user;
  id_t _uid;
  osm2rdf::osm::generic::version_t _version;
  bool _visible;
  ::util::geo::DPoint _geom;
  osm2rdf::osm::TagList _tags;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_NODE_H_
