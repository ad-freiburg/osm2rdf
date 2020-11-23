// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Relation.h"

#include <vector>

#include "osm2ttl/osm/RelationMember.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2ttl::osm::Relation::Relation(const osmium::Relation& relation) {
  _id = relation.positive_id();
  _tags = osm2ttl::osm::convertTagList(relation.tags());
  _members.reserve(relation.members().size());
  for (const auto& member : relation.members()) {
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
