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

#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/RelationMember.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2rdf::osm::Relation::Relation() {
  _id = std::numeric_limits<osm2rdf::osm::Relation::id_t>::max();
}

// ____________________________________________________________________________
osm2rdf::osm::Relation::Relation(const osmium::Relation& relation) {
  _id = relation.positive_id();
  _tags = osm2rdf::osm::convertTagList(relation.tags());
  _members.reserve(relation.cmembers().size());
  for (const auto& member : relation.cmembers()) {
    _members.emplace_back(member);
  }
}

// ____________________________________________________________________________
osm2rdf::osm::Relation::id_t osm2rdf::osm::Relation::id() const noexcept {
  return _id;
}

// ____________________________________________________________________________
const osm2rdf::osm::TagList& osm2rdf::osm::Relation::tags() const noexcept {
  return _tags;
}

// ____________________________________________________________________________
const std::vector<osm2rdf::osm::RelationMember>&
osm2rdf::osm::Relation::members() const noexcept {
  return _members;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::operator==(const Relation& other) const noexcept {
  return _id == other._id && _members == other._members && _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Relation::operator!=(
    const osm2rdf::osm::Relation& other) const noexcept {
  return !(*this == other);
}
