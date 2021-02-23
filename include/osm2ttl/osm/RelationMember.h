// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_RELATIONMEMBER_H_
#define OSM2TTL_OSM_RELATIONMEMBER_H_

#include <string>

#include "boost/serialization/access.hpp"
#include "boost/serialization/nvp.hpp"
#include "osmium/osm/relation.hpp"

namespace osm2ttl::osm {

enum class RelationMemberType : uint8_t { UNKNOWN, NODE, RELATION, WAY };

class RelationMember {
 public:
  typedef uint64_t id_t;
  RelationMember();
  explicit RelationMember(const osmium::RelationMember& relationMember);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] std::string role() const noexcept;
  [[nodiscard]] osm2ttl::osm::RelationMemberType type() const noexcept;

  bool operator==(const osm2ttl::osm::RelationMember& other) const noexcept;
  bool operator!=(const osm2ttl::osm::RelationMember& other) const noexcept;

 protected:
  id_t _id;
  std::string _role;
  osm2ttl::osm::RelationMemberType _type;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_role", _role);
    ar& boost::serialization::make_nvp("_type", _type);
  }
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_RELATIONMEMBER_H_
