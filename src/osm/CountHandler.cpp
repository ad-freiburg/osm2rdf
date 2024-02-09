// Copyright 2024, University of Freiburg
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

#include "osm2rdf/osm/CountHandler.h"

#include <iostream>

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::prepare_for_lookup() { _firstPassDone = true; }

// ----------------------------------------------------------------------------
void osm2rdf::osm::CountHandler::changeset(
    const osmium::Changeset& /*unused*/) {
  if (_firstPassDone) {
    return;
  }
  _numChangesets++;
}
// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::node(const osmium::Node& node){
  if (_firstPassDone || node.tags().empty())  {
    return;
  }
  _numNodes++;
}

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::relation(const osmium::Relation& rel) {
  if (_firstPassDone || rel.tags().empty()) {
    return;
  }
  _numRelations++;
}

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::way(const osmium::Way& way) {
  if (_firstPassDone || way.tags().empty()) {
    return;
  }
  _numWays++;
}

// ----------------------------------------------------------------------------
size_t osm2rdf::osm::CountHandler::numChangesets() const {
  return _numChangesets;
}

// ____________________________________________________________________________
size_t osm2rdf::osm::CountHandler::numNodes() const { return _numNodes; }

// ____________________________________________________________________________
size_t osm2rdf::osm::CountHandler::numRelations() const {
  return _numRelations;
}

// ____________________________________________________________________________
size_t osm2rdf::osm::CountHandler::numWays() const {
  return _numWays;
}
