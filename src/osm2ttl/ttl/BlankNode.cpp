// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/BlankNode.h"

#include <cstdint>

// ____________________________________________________________________________
uint64_t osm2ttl::ttl::BlankNode::_idCounter;

// ____________________________________________________________________________
osm2ttl::ttl::BlankNode::BlankNode() {
  _id = _idCounter++;
}

// ____________________________________________________________________________
uint64_t osm2ttl::ttl::BlankNode::getId() const {
  return _id;
}
