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

#include "osm2rdf/osm/RelationMember.h"

#include <string>

#include "osmium/osm/item_type.hpp"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2rdf::osm::RelationMember::RelationMember() {
  _id = std::numeric_limits<osm2rdf::osm::RelationMember::id_t>::max();
  _type = osm2rdf::osm::RelationMemberType::UNKNOWN;
}
// ____________________________________________________________________________
osm2rdf::osm::RelationMember::RelationMember(
    const osmium::RelationMember& relationMember) {
  _id = relationMember.positive_ref();
  _role = std::string(relationMember.role());
  if (_role.empty()) {
    _role = "member";
  }
  switch (relationMember.type()) {
    case osmium::item_type::node:
      _type = osm2rdf::osm::RelationMemberType::NODE;
      break;
    case osmium::item_type::relation:
      _type = osm2rdf::osm::RelationMemberType::RELATION;
      break;
    case osmium::item_type::way:
      _type = osm2rdf::osm::RelationMemberType::WAY;
      break;
    default:
      _type = osm2rdf::osm::RelationMemberType::UNKNOWN;
  }
}

// ____________________________________________________________________________
osm2rdf::osm::RelationMember::id_t osm2rdf::osm::RelationMember::id()
    const noexcept {
  return _id;
}

// ____________________________________________________________________________
std::string osm2rdf::osm::RelationMember::role() const noexcept {
  return _role;
}

// ____________________________________________________________________________
osm2rdf::osm::RelationMemberType osm2rdf::osm::RelationMember::type()
    const noexcept {
  return _type;
}

// ____________________________________________________________________________
bool osm2rdf::osm::RelationMember::operator==(
    const RelationMember& other) const noexcept {
  return _id == other._id && _type == other._type && _role == other._role;
}

// ____________________________________________________________________________
bool osm2rdf::osm::RelationMember::operator!=(
    const osm2rdf::osm::RelationMember& other) const noexcept {
  return !(*this == other);
}
