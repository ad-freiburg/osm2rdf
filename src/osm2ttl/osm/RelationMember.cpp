// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/RelationMember.h"

#include <string>

#include "osmium/osm/item_type.hpp"
#include "osmium/osm/relation.hpp"

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
      _type = "node";
      break;
    case osmium::item_type::relation:
      _type = "relation";
      break;
    case osmium::item_type::way:
      _type = "way";
      break;
    default:
      _type = "unknown";
  }
}

// ____________________________________________________________________________
uint64_t osm2ttl::osm::RelationMember::id() const noexcept {
  return _id;
}

// ____________________________________________________________________________
std::string osm2ttl::osm::RelationMember::role() const noexcept {
  return _role;
}

// ____________________________________________________________________________
std::string osm2ttl::osm::RelationMember::type() const noexcept {
  return _type;
}
