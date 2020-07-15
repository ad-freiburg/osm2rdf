// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_RELATIONMEMBER_H_
#define OSM2TTL_OSM_RELATIONMEMBER_H_

#include <string>

#include "osmium/osm/relation.hpp"

namespace osm2ttl {
namespace osm {

enum class RelationMemberType : uint8_t {
  UNKNOWN, NODE, RELATION, WAY
};

class RelationMember {
 public:
  explicit RelationMember(const osmium::RelationMember& relationMember);
  uint64_t id() const noexcept;
  std::string role() const noexcept;
  osm2ttl::osm::RelationMemberType type() const noexcept;
 protected:
  uint64_t _id;
  std::string _role;
  osm2ttl::osm::RelationMemberType _type;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_RELATIONMEMBER_H_

