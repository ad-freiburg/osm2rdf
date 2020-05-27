// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/BlankNodeLabel.h"

#include <ostream>

// ____________________________________________________________________________
std::ostream &operator<<(std::ostream& os,
                         const osm2nt::nt::BlankNodeLabel& b) {
  return os << "_:" << b.value;
}
