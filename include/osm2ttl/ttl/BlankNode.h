// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_BLANKNODE_H_
#define OSM2TTL_TTL_BLANKNODE_H_

#include <cstdint>

namespace osm2ttl {
namespace ttl {

struct BlankNode {
 public:
  BlankNode();
  uint64_t getId() const;
 protected:
  uint64_t _id;
  static uint64_t _idCounter;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_BLANKNODE_H_
