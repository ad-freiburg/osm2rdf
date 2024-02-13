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

#include "osm2rdf/osm/Relation.h"

#include <iostream>
#include <vector>

#include "boost/geometry.hpp"
#include "boost/version.hpp"
#include "osm2rdf/osm/Generic.h"
#include "osm2rdf/osm/RelationHandler.h"
#include "osm2rdf/osm/RelationMember.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2rdf::osm::Relation::Relation() {
  _id = std::numeric_limits<osm2rdf::osm::Relation::id_t>::max();
  _hasCompleteGeometry = false;
}

// ____________________________________________________________________________
osm2rdf::osm::Relation::Relation(const osmium::Relation& relation) {
  _id = relation.positive_id();
  _timestamp = relation.timestamp().seconds_since_epoch();
  _changeset = relation.changeset();
  _user = relation.user();
  _version = relation.version();
  _visible = relation.visible();
  _tags = osm2rdf::osm::convertTagList(relation.tags());
  _members.reserve(relation.cmembers().size());
  for (const auto& member : relation.cmembers()) {
    _members.emplace_back(member);
  }
  _hasCompleteGeometry = false;
}

// ____________________________________________________________________________
osm2rdf::osm::Relation::id_t osm2rdf::osm::Relation::id() const noexcept {
  return _id;
}

// ____________________________________________________________________________
osm2rdf::osm::generic::changeset_id_t osm2rdf::osm::Relation::changeset()
    const noexcept {
  return _changeset;
}

// ____________________________________________________________________________
std::time_t osm2rdf::osm::Relation::timestamp() const noexcept {
  return _timestamp;
}

// ____________________________________________________________________________
std::string osm2rdf::osm::Relation::user() const noexcept { return _user; }

// ____________________________________________________________________________
osm2rdf::osm::generic::version_t osm2rdf::osm::Relation::version()
    const noexcept {
  return _version;
}
// ____________________________________________________________________________
bool osm2rdf::osm::Relation::visible()
    const noexcept {
  return _visible;
}

// ____________________________________________________________________________
const osm2rdf::osm::TagList& osm2rdf::osm::Relation::tags() const noexcept {
  return _tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::isArea() const noexcept {
  const auto& typeTag = _tags.find("type");
  if (typeTag != _tags.end()) {
    return typeTag->second == "multipolygon" || typeTag->second == "boundary";
  }
  return false;
}

// ____________________________________________________________________________
const std::vector<osm2rdf::osm::RelationMember>&
osm2rdf::osm::Relation::members() const noexcept {
  return _members;
}

#if BOOST_VERSION >= 107800
// ____________________________________________________________________________
bool osm2rdf::osm::Relation::hasGeometry() const noexcept {
  return !(_geom.empty());
}

// ____________________________________________________________________________
const osm2rdf::geometry::Relation& osm2rdf::osm::Relation::geom()
    const noexcept {
  return _geom;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Box& osm2rdf::osm::Relation::envelope()
    const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Polygon& osm2rdf::osm::Relation::convexHull()
    const noexcept {
  return _convexHull;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Polygon& osm2rdf::osm::Relation::orientedBoundingBox()
    const noexcept {
  return _obb;
}

// ____________________________________________________________________________
void osm2rdf::osm::Relation::buildGeometry(
    osm2rdf::osm::RelationHandler& relationHandler) {
  _hasCompleteGeometry = true;
  for (const auto& member : _members) {
    osmium::Location res;
    osm2rdf::geometry::Way way;
    std::vector<uint64_t> nodeRefs;
    switch (member.type()) {
      case RelationMemberType::WAY:
        nodeRefs = relationHandler.get_noderefs_of_way(member.id());
        if (nodeRefs.empty()) {
          _hasCompleteGeometry = false;
          break;
        }
        for (const auto& nodeRef : nodeRefs) {
          res = relationHandler.get_node_location(nodeRef);
          if (res.valid()) {
            boost::geometry::append(
                way, osm2rdf::geometry::Node{res.lon(), res.lat()});
          } else {
            _hasCompleteGeometry = false;
          }
        }
        boost::geometry::traits::emplace_back<geometry::Relation>::apply(
            _geom, std::move(way));
        break;
      case RelationMemberType::NODE:
        res = relationHandler.get_node_location(member.id());
        if (res.valid()) {
          boost::geometry::traits::emplace_back<geometry::Relation>::apply(
              _geom, osm2rdf::geometry::Node{res.lon(), res.lat()});
        } else {
          _hasCompleteGeometry = false;
        }
        break;
      case RelationMemberType::RELATION:
        // Mark relations containing relations as incomplete for now.
        _hasCompleteGeometry = false;
        break;
      default:
        break;
    }
  }
  if (!_geom.empty()) {
    boost::geometry::envelope(_geom, _envelope);
    boost::geometry::convex_hull(_geom, _convexHull);
    _obb =
        osm2rdf::osm::generic::orientedBoundingBoxFromConvexHull(_convexHull);
  } else {
    _envelope.min_corner() = geometry::Location{0, 0};
    _envelope.max_corner() = geometry::Location{0, 0};
  }
}
#endif  // BOOST_VERSION >= 107800

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::hasCompleteGeometry() const noexcept {
  return _hasCompleteGeometry;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::operator==(const Relation& other) const noexcept {
  return _id == other._id &&
#if BOOST_VERSION >= 107800
         _envelope == other._envelope &&
#endif  // BOOST_VERSION >= 107800
         _members == other._members &&
#if BOOST_VERSION >= 107800
         _geom == other._geom &&
#endif  // BOOST_VERSION >= 107800
         _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::operator!=(
    const osm2rdf::osm::Relation& other) const noexcept {
  return !(*this == other);
}
