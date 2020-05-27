// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_TRIPLE_H_
#define OSM2NT_NT_TRIPLE_H_

#include "osm2nt/nt/Object.h"
#include "osm2nt/nt/Predicate.h"
#include "osm2nt/nt/Subject.h"

namespace osm2nt {
namespace nt {

struct Triple {
 public:
  Triple(const osm2nt::nt::Subject* s, const osm2nt::nt::Predicate* p,
         const osm2nt::nt::Object* o);
  const osm2nt::nt::Object* object;
  const osm2nt::nt::Predicate* predicate;
  const osm2nt::nt::Subject* subject;
};

}  // namespace nt
}  // namespace osm2nt
std::ostream &operator<<(std::ostream& os, const osm2nt::nt::Triple& t);

#endif  // OSM2NT_NT_TRIPLE_H_
