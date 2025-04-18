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

#include <iostream>
#include <vector>

#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/RelationHandler.h"
#include "osm2rdf/osm/RelationMember.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/relation.hpp"
#include "util/geo/Geo.h"

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
  _uid = relation.uid();
  _version = relation.version();
  _visible = relation.visible();
  _tags = std::move(osm2rdf::osm::convertTagList(relation.tags()));
  _members.reserve(relation.cmembers().size());
  for (const auto& member : relation.cmembers()) {
    _members.emplace_back(member);
  }
  _hasCompleteGeometry = false;

  auto typeTag = relation.tags()["type"];
  _isArea = typeTag != nullptr && (strcmp(typeTag, "multipolygon") == 0 ||
                                   strcmp(typeTag, "boundary") == 0);
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
id_t osm2rdf::osm::Relation::uid() const noexcept { return _uid; }

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
bool osm2rdf::osm::Relation::isArea() const noexcept { return _isArea; }

// ____________________________________________________________________________
const std::vector<osm2rdf::osm::RelationMember>&
osm2rdf::osm::Relation::members() const noexcept {
  return _members;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::hasGeometry() const noexcept {
  return !(_geom.empty());
}

// ____________________________________________________________________________
const ::util::geo::DCollection& osm2rdf::osm::Relation::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
const ::util::geo::DBox& osm2rdf::osm::Relation::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
const ::util::geo::DPolygon& osm2rdf::osm::Relation::convexHull()
    const noexcept {
  return _convexHull;
}

// ____________________________________________________________________________
const ::util::geo::DPolygon& osm2rdf::osm::Relation::orientedBoundingBox()
    const noexcept {
  return _obb;
}

// ____________________________________________________________________________
const ::util::geo::DPoint osm2rdf::osm::Relation::centroid() const noexcept {
  return ::util::geo::centroid(_geom);
}

// ____________________________________________________________________________
void osm2rdf::osm::Relation::buildGeometry(
    osm2rdf::osm::RelationHandler& relationHandler) {
  _hasCompleteGeometry = true;
  for (const auto& member : _members) {
    if (member.type() == RelationMemberType::WAY) {
      const auto& nodeRefs = relationHandler.get_noderefs_of_way(member.id());
      if (nodeRefs.empty()) {
        _hasCompleteGeometry = false;
      }

      ::util::geo::DLine way;
      way.reserve(nodeRefs.size());
      for (const auto& nodeRef : nodeRefs) {
        const auto& res = relationHandler.get_node_location(nodeRef);
        if (res.valid()) {
          way.push_back({res.lon(), res.lat()});
        } else {
          _hasCompleteGeometry = false;
        }
      }

      if (way.size() > 0) _geom.push_back(way);
    } else if (member.type() == RelationMemberType::NODE) {
      const auto& res = relationHandler.get_node_location(member.id());
      if (res.valid()) {
        _geom.push_back(::util::geo::DPoint{res.lon(), res.lat()});
      } else {
        _hasCompleteGeometry = false;
      }
    } else if (member.type() == RelationMemberType::RELATION) {
      // Mark relations containing relations as incomplete for now.
      _hasCompleteGeometry = false;
    }
  }

  if (_hasCompleteGeometry && !_geom.empty()) {
    _envelope = ::util::geo::getBoundingBox(_geom);
    _convexHull = ::util::geo::convexHull(_geom);
    _obb = ::util::geo::convexHull(::util::geo::getOrientedEnvelope(_geom));
  } else {
    _envelope = {{0, 0}, {0, 0}};
    _convexHull = ::util::geo::convexHull(_envelope);
    _obb = ::util::geo::convexHull(_envelope);
  }
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::hasCompleteGeometry() const noexcept {
  return _hasCompleteGeometry;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::operator==(const Relation& other) const noexcept {
  return _id == other._id && _envelope == other._envelope &&
         _members == other._members && _geom == other._geom &&
         _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::operator!=(
    const osm2rdf::osm::Relation& other) const noexcept {
  return !(*this == other);
}
