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

TEST(GeometryHandler, addNamedAreaFromRelation) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("test-GeometryHandler", "addNode-output");
  config.cache = config.getTempPath("test-GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

  const int areaId = 43;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_tag("name", "Name"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{buffer.get<osmium::Area>(0)};
  ASSERT_FALSE(a.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(a);
  ASSERT_EQ(1, gh._spatialStorageArea.size());
  ASSERT_NE(gh._spatialStorageAreaIndex.end(),
            gh._spatialStorageAreaIndex.find(areaId));
  ASSERT_EQ(0, gh._spatialStorageAreaIndex[areaId]);

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

TEST(GeometryHandler, addNamedAreaFromWay) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("test-GeometryHandler", "addNode-output");
  config.cache = config.getTempPath("test-GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

  const int areaId = 42;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_tag("name", "Name"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{buffer.get<osmium::Area>(0)};
  ASSERT_TRUE(a.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(a);
  ASSERT_EQ(1, gh._spatialStorageArea.size());
  ASSERT_NE(gh._spatialStorageAreaIndex.end(),
            gh._spatialStorageAreaIndex.find(areaId));
  ASSERT_EQ(0, gh._spatialStorageAreaIndex[areaId]);

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

TEST(GeometryHandler, addUnnamedAreaFromRelation) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("test-GeometryHandler", "addNode-output");
  config.cache = config.getTempPath("test-GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

  const int areaId = 43;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{buffer.get<osmium::Area>(0)};

  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(a);
  ASSERT_EQ(1, gh._numUnnamedAreas);

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

TEST(GeometryHandler, addUnnamedAreaFromWay) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("test-GeometryHandler", "addNode-output");
  config.cache = config.getTempPath("test-GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

  const int areaId = 42;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{buffer.get<osmium::Area>(0)};

  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(a);
  ASSERT_EQ(0, gh._numUnnamedAreas);

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

TEST(GeometryHandler, addWay) {
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
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Way w{buffer.get<osmium::Way>(0)};

  ASSERT_EQ(0, gh._numWays);
  gh.way(w);
  ASSERT_EQ(1, gh._numWays);

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

}  // namespace osm2ttl::osm