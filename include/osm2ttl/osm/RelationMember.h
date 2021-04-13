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
