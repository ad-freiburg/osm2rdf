// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_PREDICATE_H_
#define OSM2NT_NT_PREDICATE_H_

#include <string>
#include <ostream>

namespace osm2nt {
namespace nt {

struct Predicate {
  virtual ~Predicate() {}
  virtual std::string toString() const = 0;
};

}  // namespace nt
}  // namespace osm2nt
std::ostream &operator<<(std::ostream& os, const osm2nt::nt::Predicate& p);

#endif  // OSM2NT_NT_PREDICATE_H_
