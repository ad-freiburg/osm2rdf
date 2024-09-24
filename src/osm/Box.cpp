// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2rdf/osm/Box.h"

#include "osm2rdf/osm/Generic.h"

// ____________________________________________________________________________
osm2rdf::osm::Box::Box() = default;

// ____________________________________________________________________________
osm2rdf::osm::Box::Box(const ::util::geo::DBox& box) : _geom(box) {}

// ____________________________________________________________________________
const ::util::geo::DBox osm2rdf::osm::Box::geom() const { return _geom; }

// ____________________________________________________________________________
const ::util::geo::DPolygon osm2rdf::osm::Box::convexHull() const noexcept {
  return ::util::geo::convexHull(_geom);
}

// ____________________________________________________________________________
const ::util::geo::DPolygon osm2rdf::osm::Box::orientedBoundingBox() const noexcept {
    return convexHull();
}

// ____________________________________________________________________________
const ::util::geo::DPoint osm2rdf::osm::Box::centroid() const noexcept {
  return ::util::geo::centroid(_geom);
}

// ____________________________________________________________________________
bool osm2rdf::osm::Box::operator==(
    const osm2rdf::osm::Box& other) const noexcept {
  return _geom == other._geom;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Box::operator!=(
    const osm2rdf::osm::Box& other) const noexcept {
  return !(*this == other);
}
