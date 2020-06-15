// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/BlankNodeLabel.h"

#include <sstream>

// ____________________________________________________________________________
uint64_t osm2nt::nt::BlankNodeLabel::_idCounter;

// ____________________________________________________________________________
osm2nt::nt::BlankNodeLabel::BlankNodeLabel() {
  _id = _idCounter++;
}

// ____________________________________________________________________________
std::string osm2nt::nt::BlankNodeLabel::toString() const {
  std::stringstream tmp;
  tmp << "_:" << _id;
  return tmp.str();
}
