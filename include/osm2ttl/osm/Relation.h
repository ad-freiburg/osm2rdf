// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_RELATION_H_
#define OSM2TTL_OSM_RELATION_H_

#include <vector>

#include "osm2ttl/osm/RelationMember.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/relation.hpp"

namespace osm2ttl::osm {

class Relation {
 public:
  typedef uint32_t id_t;
  explicit Relation(const osmium::Relation& relation);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] std::vector<osm2ttl::osm::RelationMember> members()
      const noexcept;
  [[nodiscard]] osm2ttl::osm::TagList tags() const noexcept;

 protected:
  id_t _id;
  std::vector<osm2ttl::osm::RelationMember> _members;
  osm2ttl::osm::TagList _tags;
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_RELATION_H_
