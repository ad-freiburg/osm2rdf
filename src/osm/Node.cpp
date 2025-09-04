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
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"
#include "util/geo/Geo.h"

// ____________________________________________________________________________
osm2rdf::osm::Node::Node() {
  _id = std::numeric_limits<osm2rdf::osm::Node::id_t>::max();
}

// ____________________________________________________________________________
osm2rdf::osm::Node::Node(const osmium::Node& node) {
  _id = node.positive_id();
  _timestamp = node.timestamp().seconds_since_epoch();
  _changeset = node.changeset();
  _user = node.user();
  _uid = node.uid();
  _version = node.version();
  _visible = node.visible();
  const auto& loc = node.location();
  _geom = ::util::geo::DPoint{loc.lon(), loc.lat()};
  _tags = osm2rdf::osm::convertTagList(node.tags());
}

// ____________________________________________________________________________
osm2rdf::osm::Node::Node(const osmium::NodeRef& nodeRef) {
  _id = nodeRef.positive_ref();
  const auto& loc = nodeRef.location();
  _geom = ::util::geo::DPoint{loc.lon(), loc.lat()};
}

// ____________________________________________________________________________
osm2rdf::osm::Node::id_t osm2rdf::osm::Node::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2rdf::osm::generic::changeset_id_t osm2rdf::osm::Node::changeset()
    const noexcept {
  return _changeset;
}

// ____________________________________________________________________________
std::time_t osm2rdf::osm::Node::timestamp() const noexcept {
  return _timestamp;
}

// ____________________________________________________________________________
std::string osm2rdf::osm::Node::user() const noexcept { return _user; }

// ____________________________________________________________________________
osm2rdf::osm::Node::id_t osm2rdf::osm::Node::uid() const noexcept {
  return _uid;
}

// ____________________________________________________________________________
osm2rdf::osm::generic::version_t osm2rdf::osm::Node::version() const noexcept {
  return _version;
}
// ____________________________________________________________________________
bool osm2rdf::osm::Node::visible() const noexcept { return _visible; }

// ____________________________________________________________________________
const ::util::geo::DPoint& osm2rdf::osm::Node::geom() const noexcept {
  return _geom;
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
