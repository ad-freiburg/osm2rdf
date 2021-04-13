// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/osm/Box.h"

#include "osm2ttl/geometry/Box.h"

// ____________________________________________________________________________
osm2ttl::osm::Box::Box() = default;

// ____________________________________________________________________________
osm2ttl::osm::Box::Box(const osm2ttl::geometry::Box& box) : _geom(box) {}

// ____________________________________________________________________________
osm2ttl::geometry::Box osm2ttl::osm::Box::geom() const { return _geom; }

// ____________________________________________________________________________
bool osm2ttl::osm::Box::operator==(
    const osm2ttl::osm::Box& other) const noexcept {
  return _geom == other._geom;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Box::operator!=(
    const osm2ttl::osm::Box& other) const noexcept {
  return !(*this == other);
}
