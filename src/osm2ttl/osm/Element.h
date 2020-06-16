// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_ELEMENT_H_
#define OSM2TTL_OSM_ELEMENT_H_

#include <string>

#include "osmium/osm/box.hpp"
#include "osmium/osm/entity_bits.hpp"
#include "osmium/osm/types.hpp"

namespace osm2ttl {
namespace osm {

struct Element {
  osmium::Box box;
  osmium::osm_entity_bits::type objType;
  osmium::unsigned_object_id_type objId;
  char tagAdministrationLevel;

  bool contains(const Element& other) const;
  bool intersects(const Element& other) const;
  bool vagueContains(const Element& other) const;
  bool vagueIntersects(const Element& other) const;
  double vagueArea() const;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_ELEMENT_H_
