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

#include "osm2rdf/osm/Node.h"

#include "boost/geometry.hpp"
#include "boost/geometry/algorithms/envelope.hpp"
#include "osm2rdf/geometry/Box.h"
#include "osm2rdf/geometry/Global.h"
#include "osm2rdf/geometry/Polygon.h"
#include "osm2rdf/osm/Generic.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"

// ____________________________________________________________________________
osm2rdf::osm::Node::Node() {
  _id = std::numeric_limits<osm2rdf::osm::Node::id_t>::max();
}

// ____________________________________________________________________________
osm2rdf::osm::Node::Node(const osmium::Node& node) {
  _id = node.positive_id();
  _timestamp = node.timestamp().seconds_since_epoch();
  const auto& loc = node.location();
  _geom = osm2rdf::geometry::Location(loc.lon(), loc.lat());
  _tags = osm2rdf::osm::convertTagList(node.tags());
}

// ____________________________________________________________________________
osm2rdf::osm::Node::Node(const osmium::NodeRef& nodeRef) {
  _id = nodeRef.positive_ref();
  const auto& loc = nodeRef.location();
  _geom = osm2rdf::geometry::Location(loc.lon(), loc.lat());
}

// ____________________________________________________________________________
osm2rdf::osm::Node::id_t osm2rdf::osm::Node::id() const noexcept { return _id; }

// ____________________________________________________________________________
std::time_t osm2rdf::osm::Node::timestamp() const noexcept {
  return _timestamp;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Location& osm2rdf::osm::Node::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
osm2rdf::geometry::Box osm2rdf::osm::Node::envelope() const noexcept {
  osm2rdf::geometry::Box envelope;
  boost::geometry::envelope(geom(), envelope);
  return envelope;
}

// ____________________________________________________________________________
osm2rdf::geometry::Polygon osm2rdf::osm::Node::convexHull() const noexcept {
  return osm2rdf::osm::generic::boxToPolygon(envelope());
}

// ____________________________________________________________________________
osm2rdf::geometry::Polygon osm2rdf::osm::Node::orientedBoundingBox()
    const noexcept {
  return convexHull();
}

// ____________________________________________________________________________
const osm2rdf::osm::TagList& osm2rdf::osm::Node::tags() const noexcept {
  return _tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Node::operator==(
    const osm2rdf::osm::Node& other) const noexcept {
  return _id == other._id && _geom == other._geom && _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Node::operator!=(
    const osm2rdf::osm::Node& other) const noexcept {
  return !(*this == other);
}
