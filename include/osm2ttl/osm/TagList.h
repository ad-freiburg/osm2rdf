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

#ifndef OSM2TTL_OSM_TAGLIST_H_
#define OSM2TTL_OSM_TAGLIST_H_

#include <string>
#include <unordered_map>

#include "boost/serialization/unordered_map.hpp"
#include "osmium/tags/taglist.hpp"

namespace osm2ttl::osm {

typedef std::unordered_map<std::string, std::string> TagList;

// Convert an osmium::TagList into a osm2ttl::osm::TagList
osm2ttl::osm::TagList convertTagList(const osmium::TagList& tagList);

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_TAGLIST_H_
