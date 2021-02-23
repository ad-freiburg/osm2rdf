// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/CacheFile.h"

#include "gtest/gtest.h"

namespace osm2ttl::util {

TEST(UTIL_CacheFile, constructorAndAutoRemove) {
  std::filesystem::path location{"/tmp/dummy"};

  ASSERT_FALSE(std::filesystem::exists(location));
  {
    osm2ttl::util::CacheFile cf(location);
    ASSERT_NE(-1, cf.fileDescriptor());
    ASSERT_TRUE(std::filesystem::exists(location));
  }
  ASSERT_FALSE(std::filesystem::exists(location));
}

}  // namespace osm2ttl::util