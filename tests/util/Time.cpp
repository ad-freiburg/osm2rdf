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

#include "osm2rdf/util/Time.h"

#include <ctime>

#include "gtest/gtest.h"

namespace osm2rdf::util {

// ____________________________________________________________________________
TEST(UTIL_Time, currentTimeFormattedStructure) {
  const std::string time = osm2rdf::util::currentTimeFormatted();
  ASSERT_EQ(26, time.size());
  ASSERT_EQ('[', time[0]);
  ASSERT_EQ('-', time[5]);
  ASSERT_EQ('-', time[8]);
  ASSERT_EQ(' ', time[11]);
  ASSERT_EQ(':', time[14]);
  ASSERT_EQ(':', time[17]);
  ASSERT_EQ('.', time[20]);
  ASSERT_EQ(']', time[24]);
  ASSERT_EQ(' ', time[25]);
}

// ____________________________________________________________________________
TEST(UTIL_Time, formattedTimeSpacer) {
  const std::string time = osm2rdf::util::formattedTimeSpacer;
  ASSERT_EQ(26, time.size());
  for (const auto& c : time) {
    ASSERT_EQ(' ', c);
  }
}

// Test that `secondsToUtc` gives the expected fields for a few edge dates.
TEST(UTIL_Time, secondsToUtcEdgeDates) {
  auto expectUtc = [](std::time_t seconds, int64_t year, unsigned month,
                      unsigned day, unsigned hour, unsigned minute,
                      unsigned second) {
    const UtcTime t = secondsToUtc(seconds);
    EXPECT_EQ(year, t.year) << seconds;
    EXPECT_EQ(month, t.month) << seconds;
    EXPECT_EQ(day, t.day) << seconds;
    EXPECT_EQ(hour, t.hour) << seconds;
    EXPECT_EQ(minute, t.minute) << seconds;
    EXPECT_EQ(second, t.second) << seconds;
  };
  // The epoch, and the last second before it.
  expectUtc(0, 1970, 1, 1, 0, 0, 0);
  expectUtc(-1, 1969, 12, 31, 23, 59, 59);
  // A leap day and the day after it (2000 is a leap year, 1900 is not).
  expectUtc(951782400, 2000, 2, 29, 0, 0, 0);
  expectUtc(951868800, 2000, 3, 1, 0, 0, 0);
  // The last second of a year.
  expectUtc(1767225599, 2025, 12, 31, 23, 59, 59);
  // The timestamp of the test of `writeSecondsAsISO`.
  expectUtc(1555936496, 2019, 4, 22, 12, 34, 56);
}

// Test that `secondsToUtc` agrees with `gmtime_r` over a range of more than
// 300 years around 1970.
TEST(UTIL_Time, secondsToUtcAgreesWithGmtime) {
  for (int64_t seconds = -5'000'000'000; seconds < 5'000'000'000;
       seconds += 9'973) {
    const auto time = static_cast<std::time_t>(seconds);
    struct tm expected;
    gmtime_r(&time, &expected);
    const UtcTime t = secondsToUtc(time);
    ASSERT_EQ(expected.tm_year + 1900, t.year) << seconds;
    ASSERT_EQ(static_cast<unsigned>(expected.tm_mon + 1), t.month) << seconds;
    ASSERT_EQ(static_cast<unsigned>(expected.tm_mday), t.day) << seconds;
    ASSERT_EQ(static_cast<unsigned>(expected.tm_hour), t.hour) << seconds;
    ASSERT_EQ(static_cast<unsigned>(expected.tm_min), t.minute) << seconds;
    ASSERT_EQ(static_cast<unsigned>(expected.tm_sec), t.second) << seconds;
  }
}

}  // namespace osm2rdf::util