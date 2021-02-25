// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/Time.h"

#include "gtest/gtest.h"

namespace osm2ttl::util {

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

TEST(UTIL_Time, formattedTimeSpacer) {
  const std::string time = osm2ttl::util::formattedTimeSpacer;
  ASSERT_EQ(26, time.size());
  for (const auto& c : time) {
    ASSERT_EQ(' ', c);
  }
}

}  // namespace osm2ttl::util