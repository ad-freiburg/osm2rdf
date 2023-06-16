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

#ifndef OSM2RDF_UTIL_RAM_H_
#define OSM2RDF_UTIL_RAM_H_

#include <unistd.h>

#include <cstdint>

namespace osm2rdf::util::ram {

constexpr int64_t KILO = 1024;
constexpr int64_t MEGA = KILO * KILO;
constexpr int64_t GIGA = KILO * MEGA;

// ____________________________________________________________________________
inline int64_t available() {
  return sysconf(_SC_AVPHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
}

// ____________________________________________________________________________
inline int64_t physPages() {
  return sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
}

}  // namespace osm2rdf::util::ram

#endif  // OSM2RDF_UTIL_RAM_H_
