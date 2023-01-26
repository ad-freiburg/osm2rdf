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

#include "boost/serialization/nvp.hpp"
#include "osm2rdf/geometry/Box.h"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/geometry/Polygon.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"

namespace osm2rdf::osm {

class Node {
 public:
  typedef uint64_t id_t;
  explicit Node();
  explicit Node(const osmium::Node& node);
  explicit Node(const osmium::NodeRef& nodeRef);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] osm2rdf::geometry::Box envelope() const noexcept;
  [[nodiscard]] const osm2rdf::geometry::Location& geom() const noexcept;
  [[nodiscard]] const osm2rdf::osm::TagList& tags() const noexcept;
  [[nodiscard]] osm2rdf::geometry::Polygon convexHull() const noexcept;
  [[nodiscard]] osm2rdf::geometry::Polygon orientedBoundingBox() const noexcept;

  bool operator==(const osm2rdf::osm::Node& other) const noexcept;
  bool operator!=(const osm2rdf::osm::Node& other) const noexcept;

 protected:
  id_t _id;
  osm2rdf::geometry::Location _geom;
  osm2rdf::osm::TagList _tags;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_tags", _tags);
  }
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_NODE_H_
