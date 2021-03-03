// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include "boost/archive/binary_iarchive.hpp"
#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, constructor) {
  osm2ttl::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_GeometryHandler", "constructor-output");
  config.cache =
      config.getTempPath("TEST_OSM_GeometryHandler", "constructor-cache");
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

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addNamedAreaFromRelation) {
  osm2ttl::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addNamedAreaFromRelation-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addNamedAreaFromRelation-cache");
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
  const osm2ttl::osm::Area src{buffer.get<osmium::Area>(0)};
  ASSERT_FALSE(src.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(src);
  ASSERT_EQ(1, gh._spatialStorageArea.size());
  ASSERT_NE(gh._spatialStorageAreaIndex.end(),
            gh._spatialStorageAreaIndex.find(areaId));
  ASSERT_EQ(0, gh._spatialStorageAreaIndex[areaId]);

  // Compare stored area with original
  const auto& dst = gh._spatialStorageArea[0];
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() == std::get<2>(dst));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addNamedAreaFromWay) {
  osm2ttl::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addNamedAreaFromWay-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addNamedAreaFromWay-cache");
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
  const osm2ttl::osm::Area src{buffer.get<osmium::Area>(0)};
  ASSERT_TRUE(src.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(src);
  ASSERT_EQ(1, gh._spatialStorageArea.size());
  ASSERT_NE(gh._spatialStorageAreaIndex.end(),
            gh._spatialStorageAreaIndex.find(areaId));
  ASSERT_EQ(0, gh._spatialStorageAreaIndex[areaId]);

  // Compare stored area with original
  const auto& dst = gh._spatialStorageArea[0];
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() == std::get<2>(dst));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addUnnamedAreaFromRelation) {
  osm2ttl::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addUnnamedAreaFromRelation-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addUnnamedAreaFromRelation-cache");
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
  const osm2ttl::osm::Area src{buffer.get<osmium::Area>(0)};

  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(src);
  ASSERT_EQ(1, gh._numUnnamedAreas);

  // Read area from dump and compare
  osm2ttl::osm::SpatialAreaValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "areas_unnamed"),
                    std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  ia >> dst;
  ifs.close();

  // Compare stored area with original
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() == std::get<2>(dst));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addUnnamedAreaFromWay) {
  osm2ttl::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addUnnamedAreaFromWay-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addUnnamedAreaFromWay-cache");
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
  const osm2ttl::osm::Area src{buffer.get<osmium::Area>(0)};

  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(src);
  ASSERT_EQ(0, gh._numUnnamedAreas);

  // Read area from dump and compare
  osm2ttl::osm::SpatialAreaValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "areas_unnamed"),
                    std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  // No area is stored -> expect an exception on loading
  ASSERT_THROW(ia >> dst, boost::archive::archive_exception);
  ifs.close();

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addNode) {
  osm2ttl::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-output");
  config.cache =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-cache");
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
  const osm2ttl::osm::Node src{buffer.get<osmium::Node>(0)};

  ASSERT_EQ(0, gh._numNodes);
  gh.node(src);
  ASSERT_EQ(1, gh._numNodes);

  // Read area from dump and compare
  osm2ttl::osm::SpatialNodeValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "nodes"),
                    std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  ia >> dst;
  ifs.close();

  // Compare stored area with original
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() == std::get<2>(dst));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addWay) {
  osm2ttl::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-output");
  config.cache =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-cache");
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
  const osm2ttl::osm::Way src{buffer.get<osmium::Way>(0)};

  ASSERT_EQ(0, gh._numWays);
  gh.way(src);
  ASSERT_EQ(1, gh._numWays);

  // Read area from dump and compare
  osm2ttl::osm::SpatialWayValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "ways"),
                    std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  ia >> dst;
  ifs.close();

  // Compare stored area with original
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() == std::get<2>(dst));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

}  // namespace osm2ttl::osm