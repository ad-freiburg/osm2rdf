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

#include "osm2rdf/util/CacheFile.h"

#include "gtest/gtest.h"
#include "osm2rdf/config/Config.h"

namespace osm2rdf::util {

// ____________________________________________________________________________
TEST(UTIL_CacheFile, constructorAndAutoRemove) {
  osm2rdf::config::Config config;
  std::filesystem::path location{config.getTempPath(
      "UTIL_CacheFile_constructorAndAutoRemove", "constructor-output")};

  ASSERT_FALSE(std::filesystem::exists(location));
  {
    osm2rdf::util::CacheFile cf(location);
    ASSERT_NE(-1, cf.fileDescriptor());
    ASSERT_TRUE(std::filesystem::exists(location));
  }
  ASSERT_FALSE(std::filesystem::exists(location));
}

// ____________________________________________________________________________
TEST(UTIL_CacheFile, close) {
  osm2rdf::config::Config config;
  std::filesystem::path location{
      config.getTempPath("UTIL_CacheFile_close", "constructor-output")};

  ASSERT_FALSE(std::filesystem::exists(location));
  {
    osm2rdf::util::CacheFile cf(location);
    ASSERT_NE(-1, cf.fileDescriptor());
    ASSERT_TRUE(std::filesystem::exists(location));
    cf.close();
    ASSERT_EQ(-1, cf.fileDescriptor());
    cf.close();
    ASSERT_EQ(-1, cf.fileDescriptor());
  }
  ASSERT_FALSE(std::filesystem::exists(location));
}

// ____________________________________________________________________________
TEST(UTIL_CacheFile, remove) {
  osm2rdf::config::Config config;
  std::filesystem::path location{
      config.getTempPath("UTIL_CacheFile_remove", "constructor-output")};

  ASSERT_FALSE(std::filesystem::exists(location));
  {
    osm2rdf::util::CacheFile cf(location);
    ASSERT_NE(-1, cf.fileDescriptor());
    ASSERT_TRUE(std::filesystem::exists(location));
    cf.close();
    cf.remove();
    ASSERT_FALSE(std::filesystem::exists(location));
  }
  ASSERT_FALSE(std::filesystem::exists(location));
}

}  // namespace osm2rdf::util