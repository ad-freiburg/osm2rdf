// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_RAM_H_
#define OSM2TTL_UTIL_RAM_H_

#include <unistd.h>

#include <cstdint>

namespace osm2ttl::util::ram {

constexpr int64_t KILO = 1024;
constexpr int64_t MEGA = KILO * KILO;
constexpr int64_t GIGA = KILO * MEGA;

// ____________________________________________________________________________
int64_t available() {
  return sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
}

// ____________________________________________________________________________
int64_t physPages() {
  return sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
}


}  // namespace osm2ttl::util::ram

#endif  // OSM2TTL_UTIL_RAM_H_
