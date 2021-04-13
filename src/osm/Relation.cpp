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

#include "osm2ttl/osm/Relation.h"

#include <vector>

#include "osm2ttl/osm/RelationMember.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2ttl::osm::Relation::Relation() {
  _id = std::numeric_limits<osm2ttl::osm::Relation::id_t>::max();
}

// ____________________________________________________________________________
osm2ttl::osm::Relation::Relation(const osmium::Relation& relation) {
  _id = relation.positive_id();
  _tags = osm2ttl::osm::convertTagList(relation.tags());
  _members.reserve(relation.cmembers().size());
  for (const auto& member : relation.cmembers()) {
    _members.emplace_back(member);
  }
}

// ____________________________________________________________________________
osm2ttl::osm::Relation::id_t osm2ttl::osm::Relation::id() const noexcept {
  return _id;
}

// ____________________________________________________________________________
osm2ttl::osm::TagList osm2ttl::osm::Relation::tags() const noexcept {
  return _tags;
}

// ____________________________________________________________________________
std::vector<osm2ttl::osm::RelationMember> osm2ttl::osm::Relation::members()
    const noexcept {
  return _members;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Relation::operator==(const Relation& other) const noexcept {
  return _id == other._id && _members == other._members && _tags == other._tags;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Relation::operator!=(
    const osm2ttl::osm::Relation& other) const noexcept {
  return !(*this == other);
}
