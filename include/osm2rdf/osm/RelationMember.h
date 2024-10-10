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

#ifndef OSM2RDF_OSM_RELATIONMEMBER_H_
#define OSM2RDF_OSM_RELATIONMEMBER_H_

#include <string>

#include "osmium/osm/relation.hpp"

namespace osm2rdf::osm {

enum class RelationMemberType : uint8_t { UNKNOWN, NODE, RELATION, WAY };

class RelationMember {
 public:
  typedef uint64_t id_t;
  RelationMember();
  explicit RelationMember(const osmium::RelationMember& relationMember);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] std::string role() const noexcept;
  [[nodiscard]] osm2rdf::osm::RelationMemberType type() const noexcept;

  bool operator==(const osm2rdf::osm::RelationMember& other) const noexcept;
  bool operator!=(const osm2rdf::osm::RelationMember& other) const noexcept;

 protected:
  id_t _id;
  std::string _role;
  osm2rdf::osm::RelationMemberType _type;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_RELATIONMEMBER_H_
