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

#include "osm2ttl/util/Time.h"

#include "gtest/gtest.h"

namespace osm2ttl::util {

// ____________________________________________________________________________
TEST(UTIL_Time, currentTimeFormattedStructure) {
  const std::string time = osm2ttl::util::currentTimeFormatted();
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
  const std::string time = osm2ttl::util::formattedTimeSpacer;
  ASSERT_EQ(26, time.size());
  for (const auto& c : time) {
    ASSERT_EQ(' ', c);
  }
}

}  // namespace osm2ttl::util