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

#include <iostream>

#include "osm2rdf/osm/CountHandler.h"

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::prepare_for_lookup() { _firstPassDone = true; }

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::node(const osmium::Node& node) {
  if (node.positive_id() < _minId) _minId = node.positive_id();
  if (node.positive_id() > _maxId) _maxId = node.positive_id();
  if (_firstPassDone || (!_config.addUntaggedNodes && node.tags().empty())) {
    return;
  }
  _numNodes++;
}

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::relation(const osmium::Relation& rel) {
  if (_firstPassDone || (!_config.addUntaggedRelations && rel.tags().empty())) {
    return;
  }
  _numRelations++;
}

// ____________________________________________________________________________
void osm2rdf::osm::CountHandler::way(const osmium::Way& way) {
  if (_firstPassDone || (!_config.addUntaggedWays && way.tags().empty())) {
    return;
  }
  _numWays++;
}

// ____________________________________________________________________________
size_t osm2rdf::osm::CountHandler::numNodes() const { return _numNodes; }

// ____________________________________________________________________________
size_t osm2rdf::osm::CountHandler::numRelations() const {
  return _numRelations;
}

// ____________________________________________________________________________
size_t osm2rdf::osm::CountHandler::numWays() const { return _numWays; }
