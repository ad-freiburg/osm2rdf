// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_BLANKNODE_H_
#define OSM2NT_NT_BLANKNODE_H_

#include <cstdint>

namespace osm2nt {
namespace nt {

struct BlankNode {
 public:
  BlankNode();
  std::uint64_t getId() const;
 protected:
  std::uint64_t _id;
  static std::uint64_t _idCounter;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_BLANKNODE_H_
