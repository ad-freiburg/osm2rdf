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

#ifndef OSM2RDF_OSM_TAGLIST_H_
#define OSM2RDF_OSM_TAGLIST_H_

#include <string>
#include <unordered_map>

#if BOOST_VERSION >= 107400 && BOOST_VERSION < 107500
#include "boost/serialization/library_version_type.hpp"
#endif
#include "boost/serialization/unordered_map.hpp"
#include "osmium/tags/taglist.hpp"

namespace osm2rdf::osm {

typedef std::unordered_map<std::string, std::string> TagList;

// Convert an osmium::TagList into a osm2rdf::osm::TagList
osm2rdf::osm::TagList convertTagList(const osmium::TagList& tagList);

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_TAGLIST_H_
