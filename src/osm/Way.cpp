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

#include <vector>

#include "boost/geometry.hpp"
#include "osm2rdf/geometry/Way.h"
#include "osm2rdf/osm/Box.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/TagList.h"
#include "osm2rdf/osm/Way.h"
#include "osmium/osm/way.hpp"

using osm2rdf::geometry::Location;

// ____________________________________________________________________________
osm2rdf::osm::Way::Way() {
  _id = std::numeric_limits<osm2rdf::osm::Way::id_t>::max();
}

// ____________________________________________________________________________
osm2rdf::osm::Way::Way(const osmium::Way& way) {
  _id = way.positive_id();
  _tags = osm2rdf::osm::convertTagList(way.tags());
  _nodes.reserve(way.nodes().size());
  _geom.reserve(way.nodes().size());
  for (const auto& nodeRef : way.nodes()) {
    _nodes.emplace_back(nodeRef);
    boost::geometry::append(_geom, Location{nodeRef.lon(), nodeRef.lat()});
  }
  boost::geometry::unique(_geom);
  boost::geometry::envelope(_geom, _envelope);
}

// ____________________________________________________________________________
osm2rdf::osm::Way::id_t osm2rdf::osm::Way::id() const noexcept { return _id; }

// ____________________________________________________________________________
const osm2rdf::osm::TagList& osm2rdf::osm::Way::tags() const noexcept {
  return _tags;
}

// ____________________________________________________________________________
const std::vector<osm2rdf::osm::Node>& osm2rdf::osm::Way::nodes()
    const noexcept {
  return _nodes;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Way& osm2rdf::osm::Way::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Box& osm2rdf::osm::Way::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::closed() const noexcept {
  return boost::geometry::equals(_nodes.front().geom(), _nodes.back().geom());
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::operator==(
    const osm2rdf::osm::Way& other) const noexcept {
  return _id == other._id && _envelope == other._envelope &&
         _nodes == other._nodes && _geom == other._geom && _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::operator!=(
    const osm2rdf::osm::Way& other) const noexcept {
  return !(*this == other);
}
