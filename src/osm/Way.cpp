// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/osm/Way.h"

#include <vector>

#include "boost/geometry.hpp"
#include "osm2ttl/geometry/Way.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/way.hpp"

// ____________________________________________________________________________
osm2ttl::osm::Way::Way() {
  _id = std::numeric_limits<osm2ttl::osm::Way::id_t>::max();
}

// ____________________________________________________________________________
osm2ttl::osm::Way::Way(const osmium::Way& way) {
  _id = way.positive_id();
  _tags = osm2ttl::osm::convertTagList(way.tags());
  _nodes.reserve(way.nodes().size());
  for (const auto& nodeRef : way.nodes()) {
    _nodes.emplace_back(nodeRef);
    boost::geometry::append(_geom, osm2ttl::osm::Node(nodeRef).geom());
  }
  boost::geometry::unique(_geom);
  boost::geometry::envelope(_geom, _envelope);
}

// ____________________________________________________________________________
osm2ttl::osm::Way::id_t osm2ttl::osm::Way::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2ttl::osm::TagList osm2ttl::osm::Way::tags() const noexcept { return _tags; }

// ____________________________________________________________________________
std::vector<osm2ttl::osm::Node> osm2ttl::osm::Way::nodes() const noexcept {
  return _nodes;
}

// ____________________________________________________________________________
osm2ttl::geometry::Way osm2ttl::osm::Way::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
osm2ttl::geometry::Box osm2ttl::osm::Way::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::closed() const noexcept {
  return boost::geometry::equals(_nodes.front().geom(), _nodes.back().geom());
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::operator==(
    const osm2ttl::osm::Way& other) const noexcept {
  return _id == other._id && _envelope == other._envelope &&
         _nodes == other._nodes && _geom == other._geom && _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::operator!=(
    const osm2ttl::osm::Way& other) const noexcept {
  return !(*this == other);
}
