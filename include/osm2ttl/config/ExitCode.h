// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_INCLUDE_OSM2TTL_CONFIG_EXITCODE_H
#define OSM2TTL_INCLUDE_OSM2TTL_CONFIG_EXITCODE_H

namespace osm2ttl::config {

enum ExitCode {
  SUCCESS = 0,
  FAILURE = 1,
  EXCEPTION,
  INPUT_MISSING = 10,
  INPUT_NOT_EXISTS,
  INPUT_IS_DIRECTORY,
  CACHE_NOT_EXISTS = 21,
  CACHE_NOT_DIRECTORY
};

}

#endif  // OSM2TTL_INCLUDE_OSM2TTL_CONFIG_EXITCODE_H
