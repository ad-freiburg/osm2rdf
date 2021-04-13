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

#ifndef OSM2TTL_UTIL_TIME_H
#define OSM2TTL_UTIL_TIME_H

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace osm2ttl::util {

inline const char* formattedTimeSpacer = "                          ";

// Return current time formatted as string.
// https://github.com/ad-freiburg/pfaedle/blob/master/src/util/log/Log.h#L42-L50
inline std::string currentTimeFormatted() {
  std::ostringstream oss;
  char tl[20];
  auto n = std::chrono::system_clock::now();
  time_t tt = std::chrono::system_clock::to_time_t(n);
  int m = std::chrono::duration_cast<std::chrono::milliseconds>(
              n - std::chrono::time_point_cast<std::chrono::seconds>(n))
              .count();
  struct tm t = *localtime(&tt);
  strftime(tl, 20, "%Y-%m-%d %H:%M:%S", &t);
  oss << "[" << tl << "." << std::setfill('0') << std::setw(3) << m << "] ";
  return oss.str();
}

}  // namespace osm2ttl::util

#endif  // OSM2TTL_UTIL_TIME_H
