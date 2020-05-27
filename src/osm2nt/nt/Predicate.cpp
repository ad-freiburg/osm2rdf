// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/Predicate.h"

#include <ostream>

// ____________________________________________________________________________
std::ostream &operator<<(std::ostream& os, const osm2nt::nt::Predicate& p) {
  return os << p.toString();
}
