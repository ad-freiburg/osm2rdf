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

#include <iostream>

#include "osm2rdf/osm/RelationHandler.h"

// ____________________________________________________________________________
osm2rdf::osm::RelationHandler::RelationHandler(
    const osm2rdf::config::Config& config) {
  _config = config;
  _locationHandler = nullptr;
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::prepare_for_lookup() {
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
    const uint64_t nodeId) const {
  return _locationHandler->get_node_location(nodeId);
}

// ____________________________________________________________________________
std::vector<uint64_t> osm2rdf::osm::RelationHandler::get_noderefs_of_way(
    const uint32_t wayId) {
  std::vector<uint64_t> ret;
  ret.reserve(_ways[wayId].size());

  for (size_t i = 0; i < _ways[wayId].size(); i++) {
    if (_ways[wayId][i] >> 31 == 0) {
      ret.push_back(_ways[wayId][i]);
    } else {
      uint64_t fullId;

      fullId = static_cast<uint64_t>(_ways[wayId][i] << 1 >> 1) << 32;
      fullId += _ways[wayId][i + 1];

      ret.push_back(fullId);
      i++;
    }
  }

  return ret;
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::relation(const osmium::Relation& relation) {
  if (_firstPassDone) {
    return;
  }

  // skip area relations completely
  for (const auto& tag : relation.tags()) {
    if (strcmp(tag.key(), "type") == 0 &&
        (strcmp(tag.value(), "multipolygon") == 0 ||
         strcmp(tag.value(), "boundary") == 0))
      return;
  }

  for (const auto& relationMember : relation.cmembers()) {
    if (relationMember.type() == osmium::item_type::way) {
      if (relationMember.positive_ref() >
          std::numeric_limits<uint32_t>::max()) {
        throw std::out_of_range("Way ID is too large");
      }
      _ways[relationMember.positive_ref()] = {};
    }
  }
}

// ____________________________________________________________________________
void osm2rdf::osm::RelationHandler::way(const osmium::Way& way) {
  if (!_firstPassDone) {
    return;
  }

  if (way.positive_id() > std::numeric_limits<uint32_t>::max()) {
    throw std::out_of_range("Way ID is too large");
  }

  if (_ways.find(way.positive_id()) != _ways.end()) {
    std::vector<uint32_t> compressed;
    compressed.reserve(way.nodes().size() / 2);

    for (const auto& nodeRef : way.nodes()) {
      size_t nid = nodeRef.positive_ref();
      if (nid <= std::numeric_limits<int32_t>::max()) {
        // handle normally
        compressed.push_back(nid);
      } else if (nid <=
                 (static_cast<size_t>(std::numeric_limits<int64_t>::max()))) {
        // handle with continuation
        int32_t lower = nid << 32 >> 32;
        int32_t upper = nid >> 32;
        upper = upper | (1 << 31);
        compressed.push_back(upper);
        compressed.push_back(lower);
      } else {
        throw std::out_of_range("Node ID is too large");
      }
    }

    _ways[way.positive_id()] = std::move(compressed);
  }
}
