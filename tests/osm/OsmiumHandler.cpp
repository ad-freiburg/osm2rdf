// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/OsmiumHandler.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

TEST(OSM_OsmiumHandler, constructor) {
  osm2ttl::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_OsmiumHandler", "constructor-output");
  config.cache =
      config.getTempPath("TEST_OSM_OsmiumHandler", "constructor-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  ASSERT_EQ(0, oh.areasSeen());
  ASSERT_EQ(0, oh.areasDumped());
  ASSERT_EQ(0, oh.areaGeometriesHandled());
  ASSERT_EQ(0, oh.nodesSeen());
  ASSERT_EQ(0, oh.nodesDumped());
  ASSERT_EQ(0, oh.nodeGeometriesHandled());
  ASSERT_EQ(0, oh.relationsSeen());
  ASSERT_EQ(0, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(0, oh.waysSeen());
  ASSERT_EQ(0, oh.waysDumped());
  ASSERT_EQ(0, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}


}  // namespace osm2ttl::osm