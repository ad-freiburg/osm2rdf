// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/Output.h"

#include "gtest/gtest.h"

namespace osm2ttl::util {

TEST(Output, partFilename) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::util::Output o{config, "test", 4};
  // Normal parts
  ASSERT_EQ("test.part_1", o.partFilename(0));
  ASSERT_EQ("test.part_2", o.partFilename(1));
  ASSERT_EQ("test.part_3", o.partFilename(2));
  ASSERT_EQ("test.part_4", o.partFilename(3));

  // Handle prefix (-1) and suffix (-2) parts
  ASSERT_EQ("test.part_0", o.partFilename(-1));
  ASSERT_EQ("test.part_5", o.partFilename(-2));
}



}