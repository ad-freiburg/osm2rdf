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
    result[key] = tag.value();
  }
  return result;
}
