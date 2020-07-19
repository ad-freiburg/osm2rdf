// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/Ram.h"

#include <unistd.h>

// ____________________________________________________________________________
int64_t osm2ttl::util::ram::available() {
  return sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
}

// ____________________________________________________________________________
int64_t osm2ttl::util::ram::physPages() {
  return sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
}
