// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/CacheFile.h"

#include "gtest/gtest.h"
#include "osm2ttl/config/Config.h"

namespace osm2ttl::util {

// ____________________________________________________________________________
TEST(UTIL_CacheFile, constructorAndAutoRemove) {
  osm2ttl::config::Config config;
  std::filesystem::path location{config.getTempPath(
      "UTIL_CacheFile_constructorAndAutoRemove", "constructor-output")};

  ASSERT_FALSE(std::filesystem::exists(location));
  {
    osm2ttl::util::CacheFile cf(location);
    ASSERT_NE(-1, cf.fileDescriptor());
    ASSERT_TRUE(std::filesystem::exists(location));
  }
  ASSERT_FALSE(std::filesystem::exists(location));
}

}  // namespace osm2ttl::util