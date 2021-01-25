// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

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
