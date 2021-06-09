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

#ifndef OSM2TTL_INCLUDE_OSM2TTL_CONFIG_EXITCODE_H
#define OSM2TTL_INCLUDE_OSM2TTL_CONFIG_EXITCODE_H

namespace osm2ttl::config {

enum ExitCode {
  SUCCESS = 0,
  FAILURE = 1,
  EXCEPTION,
  UNKNOWN_ARGUMENT,
  INPUT_MISSING = 10,
  INPUT_NOT_EXISTS,
  INPUT_IS_DIRECTORY,
  CACHE_NOT_EXISTS = 21,
  CACHE_NOT_DIRECTORY
};

}

#endif  // OSM2TTL_INCLUDE_OSM2TTL_CONFIG_EXITCODE_H
