// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_TAGLIST_H_
#define OSM2TTL_OSM_TAGLIST_H_

#include <string>
#include <unordered_map>

#include "osmium/tags/taglist.hpp"

namespace osm2ttl::osm {

typedef std::unordered_map<std::string, std::string> TagList;

osm2ttl::osm::TagList convertTagList(const osmium::TagList& tagList);

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_TAGLIST_H_
