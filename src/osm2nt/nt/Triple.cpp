// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/Triple.h"

#include <iostream>
#include <ostream>

// ____________________________________________________________________________
osm2nt::nt::Triple::Triple(const osm2nt::nt::Subject* s,
                           const osm2nt::nt::Predicate* p,
                           const osm2nt::nt::Object* o) {
  subject = s;
  predicate = p;
  object = o;
}

// ____________________________________________________________________________
std::ostream &operator<<(std::ostream& os, const osm2nt::nt::Triple& t) {
  return os << t.subject->toString() << " " << t.predicate->toString() << " "
    << t.object->toString() << " .";
}
