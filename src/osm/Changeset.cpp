// Copyright 2024, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>

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

#include "osm2rdf/osm/Changeset.h"

#include <iostream>
#include <limits>

#include "osmium/osm/changeset.hpp"

// ____________________________________________________________________________
osm2rdf::osm::Changeset::Changeset() {
  _id = std::numeric_limits<osm2rdf::osm::Changeset::id_t>::max();
}
// ____________________________________________________________________________
osm2rdf::osm::Changeset::Changeset(const osmium::Changeset& changeset)
    : Changeset() {
  _id = changeset.id();
}

// ____________________________________________________________________________
osm2rdf::osm::Changeset::id_t osm2rdf::osm::Changeset::id() const noexcept {
  return _id;
}