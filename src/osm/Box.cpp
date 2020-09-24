// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Box.h"

#include "osm2ttl/geometry/Box.h"

// ____________________________________________________________________________
osm2ttl::osm::Box::Box(const osm2ttl::geometry::Box& box) { _geom = box; }

// ____________________________________________________________________________
osm2ttl::geometry::Box osm2ttl::osm::Box::geom() const { return _geom; }
