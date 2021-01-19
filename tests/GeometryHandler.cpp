// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

TEST(GeometryHandler, constructor) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output =
      config.getTempPath("test-GeometryHandler", "constructor-output");
  config.cache =
      config.getTempPath("test-GeometryHandler", "constructor-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

TEST(GeometryHandler, addNode) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("test-GeometryHandler", "addNode-output");
  config.cache = config.getTempPath("test-GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Node n{buffer.get<osmium::Node>(0)};

  ASSERT_EQ(0, gh._numNodes);
  gh.node(n);
  ASSERT_EQ(1, gh._numNodes);

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

}