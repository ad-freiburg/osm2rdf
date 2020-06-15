// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/BlankNode.h"

#include <cstdint>

// ____________________________________________________________________________
std::uint64_t osm2nt::nt::BlankNode::_idCounter;

// ____________________________________________________________________________
osm2nt::nt::BlankNode::BlankNode() {
  _id = _idCounter++;
}

// ____________________________________________________________________________
std::uint64_t osm2nt::nt::BlankNode::getId() const {
  return _id;
}
