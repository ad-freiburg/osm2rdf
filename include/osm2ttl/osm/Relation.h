// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_RELATION_H_
#define OSM2TTL_OSM_RELATION_H_

#include <vector>

#include "osmium/osm/relation.hpp"

#include "osm2ttl/osm/RelationMember.h"
#include "osm2ttl/osm/TagList.h"

namespace osm2ttl {
namespace osm {

class Relation {
 public:
  explicit Relation(const osmium::Relation& relation);
  [[nodiscard]] uint64_t id() const noexcept;
  [[nodiscard]] std::vector<osm2ttl::osm::RelationMember> members() const noexcept;
  [[nodiscard]] osm2ttl::osm::TagList tags() const noexcept;
 protected:
  uint64_t _id;
  std::vector<osm2ttl::osm::RelationMember> _members;
  osm2ttl::osm::TagList _tags;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_RELATION_H_


