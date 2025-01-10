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

#include <vector>

#include "osm2rdf/osm/Box.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/TagList.h"
#include "osm2rdf/osm/Way.h"
#include "osmium/osm/way.hpp"

// ____________________________________________________________________________
osm2rdf::osm::Way::Way() {
  _id = std::numeric_limits<osm2rdf::osm::Way::id_t>::max();
}

// ____________________________________________________________________________
osm2rdf::osm::Way::Way(const osmium::Way& way) {
  _id = way.positive_id();
  _timestamp = way.timestamp().seconds_since_epoch();
  _changeset = way.changeset();
  _user = way.user();
  _uid = way.uid();
  _version = way.version();
  _visible = way.visible();
  _tags = std::move(osm2rdf::osm::convertTagList(way.tags()));
  _nodes.reserve(way.nodes().size());
  _geom.reserve(way.nodes().size());

  auto areaTag = way.tags()["area"];
  _hasAreaTag = areaTag == nullptr || strcmp(areaTag, "no") != 0;

  double lonMin = std::numeric_limits<double>::infinity();
  double latMin = std::numeric_limits<double>::infinity();
  double lonMax = -std::numeric_limits<double>::infinity();
  double latMax = -std::numeric_limits<double>::infinity();

  for (const auto& nodeRef : way.nodes()) {
    if (nodeRef.lon() < lonMin) {
      lonMin = nodeRef.lon();
    }
    if (nodeRef.lat() < latMin) {
      latMin = nodeRef.lat();
    }
    if (nodeRef.lon() > lonMax) {
      lonMax = nodeRef.lon();
    }
    if (nodeRef.lat() > latMax) {
      latMax = nodeRef.lat();
    }

    _nodes.emplace_back(nodeRef);

    if (_geom.empty() || (nodeRef.lon() != _geom.back().getX() ||
                          nodeRef.lat() != _geom.back().getY())) {
      _geom.push_back({nodeRef.lon(), nodeRef.lat()});
    }
  }
  _envelope = {{lonMin, latMin}, {lonMax, latMax}};
}

// ____________________________________________________________________________
void osm2rdf::osm::Way::finalize() {
  _convexHull = ::util::geo::convexHull(_geom);
  _obb = ::util::geo::convexHull(::util::geo::getOrientedEnvelope(_geom));
}

// ____________________________________________________________________________
osm2rdf::osm::Way::id_t osm2rdf::osm::Way::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2rdf::osm::generic::changeset_id_t osm2rdf::osm::Way::changeset()
    const noexcept {
  return _changeset;
}

// ____________________________________________________________________________
std::time_t osm2rdf::osm::Way::timestamp() const noexcept { return _timestamp; }

// ____________________________________________________________________________
std::string osm2rdf::osm::Way::user() const noexcept { return _user; }

// ____________________________________________________________________________
id_t osm2rdf::osm::Way::uid() const noexcept { return _uid; }

// ____________________________________________________________________________
osm2rdf::osm::generic::version_t osm2rdf::osm::Way::version() const noexcept {
  return _version;
}
// ____________________________________________________________________________
bool osm2rdf::osm::Way::visible() const noexcept { return _visible; }

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
const ::util::geo::DLine& osm2rdf::osm::Way::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
const ::util::geo::DBox& osm2rdf::osm::Way::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
const ::util::geo::DPolygon& osm2rdf::osm::Way::convexHull() const noexcept {
  return _convexHull;
}

// ____________________________________________________________________________
const ::util::geo::DPolygon& osm2rdf::osm::Way::orientedBoundingBox()
    const noexcept {
  return _obb;
}

// ____________________________________________________________________________
const ::util::geo::DPoint osm2rdf::osm::Way::centroid() const noexcept {
  return ::util::geo::centroid(_geom);
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::closed() const noexcept {
  return _nodes.front().geom() == _nodes.back().geom();
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::isArea() const noexcept {
  // See libosmium/include/osmium/area/multipolygon_manager.hpp:154
  if (_nodes.size() < 4) {
    return false;
  }
  if (!closed()) {
    return false;
  }
  return _hasAreaTag;
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
