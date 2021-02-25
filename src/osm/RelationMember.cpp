// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/RelationMember.h"

#include <string>

#include "osmium/osm/item_type.hpp"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2ttl::osm::RelationMember::RelationMember() {
  _id = std::numeric_limits<osm2ttl::osm::RelationMember::id_t>::max();
  _type = osm2ttl::osm::RelationMemberType::UNKNOWN;
}
// ____________________________________________________________________________
osm2ttl::osm::RelationMember::RelationMember(
    const osmium::RelationMember& relationMember) {
  _id = relationMember.positive_ref();
  _role = std::string(relationMember.role());
  if (_role.empty()) {
    _role = "member";
  }
  switch (relationMember.type()) {
    case osmium::item_type::node:
      _type = osm2ttl::osm::RelationMemberType::NODE;
      break;
    case osmium::item_type::relation:
      _type = osm2ttl::osm::RelationMemberType::RELATION;
      break;
    case osmium::item_type::way:
      _type = osm2ttl::osm::RelationMemberType::WAY;
      break;
    default:
      _type = osm2ttl::osm::RelationMemberType::UNKNOWN;
  }
}

// ____________________________________________________________________________
osm2ttl::osm::RelationMember::id_t osm2ttl::osm::RelationMember::id()
    const noexcept {
  return _id;
}

// ____________________________________________________________________________
std::string osm2ttl::osm::RelationMember::role() const noexcept {
  return _role;
}

// ____________________________________________________________________________
osm2ttl::osm::RelationMemberType osm2ttl::osm::RelationMember::type()
    const noexcept {
  return _type;
}

// ____________________________________________________________________________
bool osm2ttl::osm::RelationMember::operator==(
    const RelationMember& other) const noexcept {
  return _id == other._id && _type == other._type && _role == other._role;
}

// ____________________________________________________________________________
bool osm2ttl::osm::RelationMember::operator!=(
    const osm2ttl::osm::RelationMember& other) const noexcept {
  return !(*this == other);
}
