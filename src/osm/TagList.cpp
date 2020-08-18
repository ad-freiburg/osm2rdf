// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/TagList.h"

#include "osmium/tags/taglist.hpp"

// ____________________________________________________________________________
osm2ttl::osm::TagList osm2ttl::osm::convertTagList(
  const osmium::TagList& tagList) {
  osm2ttl::osm::TagList result;

  for (const auto& tag : tagList) {
    std::string key{tag.key()};
    for (size_t pos = 0; pos < key.size(); ++pos) {
      switch (key[pos]) {
        case ' ':
          key[pos] = '_';
          break;
      }
    }
    result[tag.key()] = tag.value();
  }
  return result;
}
