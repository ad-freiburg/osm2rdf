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

#ifndef OSM2RDF_UTIL_TIME_H
#define OSM2RDF_UTIL_TIME_H

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace osm2rdf::util {

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

// A point in time in UTC, broken down into its calendar fields.
struct UtcTime {
  int64_t year;
  unsigned month;  // 1 to 12
  unsigned day;    // 1 to 31
  unsigned hour;
  unsigned minute;
  unsigned second;
};

// Convert seconds since 1970-01-01T00:00:00Z to UTC calendar fields.
//
// NOTE: This replaces `gmtime_r`, which takes a global lock in glibc (also for
// UTC), so that many threads that convert timestamps at the same time
// serialize on it. The conversion is the lock-free `civil_from_days` from
// https://howardhinnant.github.io/date_algorithms.html (public domain).
inline UtcTime secondsToUtc(std::time_t seconds) {
  constexpr int64_t secondsPerDay = 86400;
  const auto t = static_cast<int64_t>(seconds);
  // Split into days and the seconds within the day. For times before 1970,
  // the remainder is negative, then move one day back (this way, no
  // intermediate value can overflow, not even for the minimum `time_t`).
  int64_t days = t / secondsPerDay;
  int64_t secondsOfDay = t % secondsPerDay;
  if (secondsOfDay < 0) {
    secondsOfDay += secondsPerDay;
    --days;
  }

  // Convert the days to a date, with eras of 400 years starting at March 1.
  const int64_t z = days + 719468;
  const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
  const auto dayOfEra = static_cast<unsigned>(z - era * 146097);
  const unsigned yearOfEra =
      (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
  const unsigned dayOfYear =
      dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
  const unsigned monthFromMarch = (5 * dayOfYear + 2) / 153;
  const unsigned day = dayOfYear - (153 * monthFromMarch + 2) / 5 + 1;
  const unsigned month =
      monthFromMarch < 10 ? monthFromMarch + 3 : monthFromMarch - 9;
  const int64_t year =
      static_cast<int64_t>(yearOfEra) + era * 400 + (month <= 2 ? 1 : 0);

  return {year,
          month,
          day,
          static_cast<unsigned>(secondsOfDay / 3600),
          static_cast<unsigned>(secondsOfDay % 3600 / 60),
          static_cast<unsigned>(secondsOfDay % 60)};
}

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_TIME_H
