// Copyright 2022, University of Freiburg
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

#include "osm2rdf/osm/RelationHandler.h"

#include <iostream>

// ____________________________________________________________________________
osm2rdf::osm::RelationHandler::RelationHandler(
    const osm2rdf::config::Config& config) {
  _config = config;
  _locationHandler = nullptr;
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::prepare_for_lookup() {
  std::sort(_relationIds.begin(), _relationIds.end());
  auto lastRelation = std::unique(_relationIds.begin(), _relationIds.end());
  _relationIds.erase(lastRelation, _relationIds.end());

  std::sort(_wayIds.begin(), _wayIds.end());
  auto lastWay = std::unique(_wayIds.begin(), _wayIds.end());
  _wayIds.erase(lastWay, _wayIds.end());

  _firstPassDone = true;
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::setLocationHandler(
    osm2rdf::osm::LocationHandler* locationHandler) {
  _locationHandler = locationHandler;
}

// ____________________________________________________________________________
bool osm2rdf::osm::RelationHandler::hasLocationHandler() const {
  return _locationHandler != nullptr;
}

// ____________________________________________________________________________
osmium::Location osm2rdf::osm::RelationHandler::get_node_location(
    const uint64_t id) const {
  return _locationHandler->get_node_location(id);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2rdf::osm::RelationHandler::get_noderefs_of_way(
    const uint64_t id) {
  return _ways[id];
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::relation(const osmium::Relation& relation) {
  if (_firstPassDone) {
    return;
  }
  for (const auto& relationMember : relation.cmembers()) {
    switch (relationMember.type()) {
      case osmium::item_type::way:
        _wayIds.push_back(relationMember.positive_ref());
        break;
      case osmium::item_type::relation:
        _relationIds.push_back(relationMember.positive_ref());
      default:
        break;
    }
  }
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::way(const osmium::Way& way) {
  if (!_firstPassDone) {
    return;
  }
  if (std::find(_wayIds.begin(), _wayIds.end(), way.positive_id()) !=
      _wayIds.end()) {
    std::vector<uint64_t> ids;
    for (const auto& nodeRef : way.nodes()) {
      ids.push_back(nodeRef.positive_ref());
    }
    _ways.insert({way.positive_id(), ids});
  }
}