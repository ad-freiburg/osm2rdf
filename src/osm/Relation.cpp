// Copyright 2020, University of Freiburg
//
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

#include "osm2rdf/osm/RelationHandler.h"
#include "osmium/osm/relation.hpp"
#include "util/geo/Geo.h"

// ____________________________________________________________________________
osm2rdf::osm::Relation::Relation() { _hasCompleteGeometry = false; }

// ____________________________________________________________________________
osm2rdf::osm::Relation::Relation(const osmium::Relation& relation)
    : _r(&relation) {
  _hasCompleteGeometry = false;

  auto typeTag = relation.tags()["type"];
  _isArea = typeTag != nullptr && (strcmp(typeTag, "multipolygon") == 0 ||
                                   strcmp(typeTag, "boundary") == 0);
}

// ____________________________________________________________________________
osm2rdf::osm::Relation::id_t osm2rdf::osm::Relation::id() const noexcept {
  return _r->positive_id();
}

// ____________________________________________________________________________
osm2rdf::osm::generic::changeset_id_t osm2rdf::osm::Relation::changeset()
    const noexcept {
  return _r->changeset();
}

// ____________________________________________________________________________
std::time_t osm2rdf::osm::Relation::timestamp() const noexcept {
  return _r->timestamp().seconds_since_epoch();
}

// ____________________________________________________________________________
std::string osm2rdf::osm::Relation::user() const noexcept { return _r->user(); }

// ____________________________________________________________________________
id_t osm2rdf::osm::Relation::uid() const noexcept { return _r->uid(); }

// ____________________________________________________________________________
osm2rdf::osm::generic::version_t osm2rdf::osm::Relation::version()
    const noexcept {
  return _r->version();
}
// ____________________________________________________________________________
bool osm2rdf::osm::Relation::visible() const noexcept { return _r->visible(); }

// ____________________________________________________________________________
const osmium::TagList& osm2rdf::osm::Relation::tags() const noexcept {
  return _r->tags();
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::isArea() const noexcept { return _isArea; }

// ____________________________________________________________________________
const osmium::RelationMemberList& osm2rdf::osm::Relation::members()
    const noexcept {
  return _r->members();
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
void osm2rdf::osm::Relation::buildGeometry(
    osm2rdf::osm::RelationHandler& relationHandler) {
  _hasCompleteGeometry = true;
  for (const auto& member : _r->members()) {
    if (member.type() == osmium::item_type::way) {
      const auto& nodeRefs =
          relationHandler.get_noderefs_of_way(member.positive_ref());
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
    } else if (member.type() == osmium::item_type::node) {
      const auto& res =
          relationHandler.get_node_location(member.positive_ref());
      if (res.valid()) {
        _geom.push_back(::util::geo::DPoint{res.lon(), res.lat()});
      } else {
        _hasCompleteGeometry = false;
      }
    } else if (member.type() == osmium::item_type::relation) {
      // Mark relations containing relations as incomplete for now.
      _hasCompleteGeometry = false;
    }
  }
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::hasCompleteGeometry() const noexcept {
  return _hasCompleteGeometry;
}
