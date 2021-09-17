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

#include "osm2rdf/osm/GeometryHandler.h"

#include <omp.h>

#include "boost/archive/binary_iarchive.hpp"
#include "boost/geometry.hpp"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, constructor) {
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_GeometryHandler", "constructor-output");
  config.cache =
      config.getTempPath("TEST_OSM_GeometryHandler", "constructor-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addNamedAreaFromRelation) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addNamedAreaFromRelation-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addNamedAreaFromRelation-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{buffer.get<osmium::Area>(0)};
  ASSERT_FALSE(src.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(src);
  ASSERT_EQ(1, gh._spatialStorageArea.size());

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
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addNamedAreaFromWay-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addNamedAreaFromWay-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{buffer.get<osmium::Area>(0)};
  ASSERT_TRUE(src.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(src);
  ASSERT_EQ(1, gh._spatialStorageArea.size());

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
TEST(OSM_GeometryHandler, addNamedAreaFromRelationWithRatios) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath(
      "TEST_OSM_GeometryHandler", "addNamedAreaFromRelationWithRatios-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addNamedAreaFromRelationWithRatios-cache");
  config.minimalAreaEnvelopeRatio = 0.75;
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  const int areaId = 43;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer1{initial_buffer_size,
                                 osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer buffer2{initial_buffer_size,
                                 osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer1, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_tag("name", "Name"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  // Rombus in square -> ratio ~0.5
  osmium::builder::add_area(buffer2, osmium::builder::attr::_id(areaId + 2),
                            osmium::builder::attr::_tag("name", "Name2"),
                            osmium::builder::attr::_outer_ring({
                                {1, {0.0, 5.0}},
                                {2, {5.0, 10.0}},
                                {3, {10.0, 5.0}},
                                {4, {5.0, 0.0}},
                                {1, {0.0, 5.0}},
                            }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src1{buffer1.get<osmium::Area>(0)};
  const osm2rdf::osm::Area src2{buffer2.get<osmium::Area>(0)};
  ASSERT_FALSE(src1.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(src1);
  ASSERT_EQ(1, gh._spatialStorageArea.size());
  gh.area(src2);
  ASSERT_EQ(1, gh._numUnnamedAreas);

  // Compare stored area with original
  const auto& dst1 = gh._spatialStorageArea[0];
  ASSERT_TRUE(src1.envelope() == std::get<0>(dst1));
  ASSERT_TRUE(src1.id() == std::get<1>(dst1));
  ASSERT_TRUE(src1.geom() == std::get<2>(dst1));

  // Read area from dump and compare
  osm2rdf::osm::SpatialAreaValue dst2;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "areas_unnamed"),
                    std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  ia >> dst2;
  ifs.close();

  // Compare stored area with original
  ASSERT_TRUE(src2.envelope() == std::get<0>(dst2));
  ASSERT_TRUE(src2.id() == std::get<1>(dst2));
  ASSERT_TRUE(src2.geom() == std::get<2>(dst2));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addNamedAreaFromWayWithRatios) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addNamedAreaFromWayWithRatios-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addNamedAreaFromWayWithRatios-cache");
  config.minimalAreaEnvelopeRatio = 0.75;
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  const int areaId = 42;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer1{initial_buffer_size,
                                 osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer buffer2{initial_buffer_size,
                                 osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer1, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_tag("name", "Name"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  // Rombus in square -> ratio ~0.5
  osmium::builder::add_area(buffer2, osmium::builder::attr::_id(areaId + 2),
                            osmium::builder::attr::_tag("name", "Name2"),
                            osmium::builder::attr::_outer_ring({
                                {1, {0.0, 5.0}},
                                {2, {5.0, 10.0}},
                                {3, {10.0, 5.0}},
                                {4, {5.0, 0.0}},
                                {1, {0.0, 5.0}},
                            }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src1{buffer1.get<osmium::Area>(0)};
  const osm2rdf::osm::Area src2{buffer2.get<osmium::Area>(0)};
  ASSERT_TRUE(src1.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(src1);
  ASSERT_EQ(1, gh._spatialStorageArea.size());
  gh.area(src2);
  ASSERT_EQ(1, gh._numUnnamedAreas);

  // Compare stored area with original
  const auto& dst1 = gh._spatialStorageArea[0];
  ASSERT_TRUE(src1.envelope() == std::get<0>(dst1));
  ASSERT_TRUE(src1.id() == std::get<1>(dst1));
  ASSERT_TRUE(src1.geom() == std::get<2>(dst1));

  // Read area from dump and compare
  osm2rdf::osm::SpatialAreaValue dst2;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "areas_unnamed"),
                    std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  ia >> dst2;
  ifs.close();

  // Compare stored area with original
  ASSERT_TRUE(src2.envelope() == std::get<0>(dst2));
  ASSERT_TRUE(src2.id() == std::get<1>(dst2));
  ASSERT_TRUE(src2.geom() == std::get<2>(dst2));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, addUnnamedAreaFromRelation) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addUnnamedAreaFromRelation-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addUnnamedAreaFromRelation-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{buffer.get<osmium::Area>(0)};

  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(src);
  ASSERT_EQ(1, gh._numUnnamedAreas);

  // Read area from dump and compare
  osm2rdf::osm::SpatialAreaValue dst;

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
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "addUnnamedAreaFromWay-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "addUnnamedAreaFromWay-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{buffer.get<osmium::Area>(0)};

  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(src);
  ASSERT_EQ(0, gh._numUnnamedAreas);

  // Read area from dump and compare
  osm2rdf::osm::SpatialAreaValue dst;

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
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-output");
  config.cache =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node src{buffer.get<osmium::Node>(0)};

  ASSERT_EQ(0, gh._numNodes);
  gh.node(src);
  ASSERT_EQ(1, gh._numNodes);

  // Read area from dump and compare
  osm2rdf::osm::SpatialNodeValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "nodes"), std::ios::binary);
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
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-output");
  config.cache =
      config.getTempPath("TEST_OSM_GeometryHandler", "addNode-cache");
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way src{buffer.get<osmium::Way>(0)};

  ASSERT_EQ(0, gh._numWays);
  gh.way(src);
  ASSERT_EQ(1, gh._numWays);

  // Read area from dump and compare
  osm2rdf::osm::SpatialWayValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "ways"), std::ios::binary);
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
TEST(OSM_GeometryHandler, prepareRTreeEmpty) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  ASSERT_EQ(0, gh._spatialIndex.size());
  gh.prepareRTree();
  ASSERT_EQ(0, gh._spatialIndex.size());

  output.flush();
  output.close();

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, prepareRTreeSimple) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();

  ASSERT_EQ(0, gh._spatialIndex.size());
  gh.prepareRTree();
  ASSERT_EQ(gh._spatialStorageArea.size(), gh._spatialIndex.size());

  std::vector<SpatialAreaValue> queryResult;
  osm2rdf::geometry::Box nodeEnvelope;

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2rdf::geometry::Location(148.05, 7.56);
  nodeEnvelope.max_corner() = osm2rdf::geometry::Location(148.05, 7.56);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));
  ASSERT_EQ(0, queryResult.size());

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2rdf::geometry::Location(45.00, 8.00);
  nodeEnvelope.max_corner() = osm2rdf::geometry::Location(45.00, 8.00);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));

  // 24, 28
  ASSERT_EQ(2, queryResult.size());

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2rdf::geometry::Location(48.05, 7.56);
  nodeEnvelope.max_corner() = osm2rdf::geometry::Location(48.05, 7.56);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));
  // 22, 24, 28
  ASSERT_EQ(3, queryResult.size());

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2rdf::geometry::Location(40.05, 7.56);
  nodeEnvelope.max_corner() = osm2rdf::geometry::Location(40.05, 7.56);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));
  // 24, 26, 28
  ASSERT_EQ(3, queryResult.size());

  output.flush();
  output.close();

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, prepareDAGEmpty) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();

  ASSERT_EQ(0, gh._directedAreaGraph.getNumVertices());
  ASSERT_EQ(0, gh._directedAreaGraph.getNumEdges());
  gh.prepareDAG();
  ASSERT_EQ(0, gh._directedAreaGraph.getNumVertices());
  ASSERT_EQ(0, gh._directedAreaGraph.getNumEdges());

  output.flush();
  output.close();

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, prepareDAGSimple) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
        28
         |
        24
       /  \
     22    26
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();

  ASSERT_EQ(0, gh._directedAreaGraph.getNumVertices());
  ASSERT_EQ(0, gh._directedAreaGraph.getNumEdges());
  gh.prepareDAG();
  ASSERT_EQ(4, gh._directedAreaGraph.getNumVertices());
  ASSERT_EQ(3, gh._directedAreaGraph.getNumEdges());

  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(22));
  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(26));
  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{28},
            gh._directedAreaGraph.getEdges(24));

  output.flush();
  output.close();

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNamedAreaRelationsEmpty) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpNamedAreaRelations();

  output.flush();
  output.close();

  ASSERT_EQ("", coutBuffer.str());

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNamedAreaRelationsSimple) {
#ifdef _OPENMP
  const auto numThreads = omp_get_thread_num();
  omp_set_num_threads(1);
#endif

  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(22));
  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(26));
  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{28},
            gh._directedAreaGraph.getEdges(24));

  ASSERT_EQ(2, gh._directedAreaGraph.getEdgesFast(22).size());
  ASSERT_EQ(1, gh._directedAreaGraph.getEdgesFast(24).size());
  ASSERT_EQ(2, gh._directedAreaGraph.getEdgesFast(26).size());
  ASSERT_EQ(0, gh._directedAreaGraph.getEdgesFast(28).size());

  gh.dumpNamedAreaRelations();

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 11)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 11)));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
#ifdef _OPENMP
  omp_set_num_threads(numThreads);
#endif
}

#ifdef _OPENMP
// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNamedAreaRelationsSimpleOpenMP) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(22));
  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(26));
  ASSERT_EQ(std::vector<osm2rdf::osm::Area::id_t>{28},
            gh._directedAreaGraph.getEdges(24));

  gh.dumpNamedAreaRelations();

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 11)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, 11)));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

#endif

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, noAreaGeometricRelations) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noAreaGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpUnnamedAreaRelations();

  output.flush();
  output.close();

  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "Skipping contains relation for unnamed areas ... disabled"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsEmpty1) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpUnnamedAreaRelations();

  output.flush();
  output.close();

  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr(
          "Skipping contains relation for unnamed areas ... no unnamed area"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsEmpty2) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpUnnamedAreaRelations();

  output.flush();
  output.close();

  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr(
          "Skipping contains relation for unnamed areas ... no unnamed area"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsSimpleIntersects) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  // Unnamed area not from any way.
  osmium::builder::add_area(osmiumBuffer5, osmium::builder::attr::_id(31),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.1, 7.56}},
                                {2, {40.1, 7.60}},
                                {3, {48.1, 7.60}},
                                {4, {48.1, 7.56}},
                                {1, {40.1, 7.56}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(osm2rdf::osm::Area(osmiumBuffer5.get<osmium::Area>(0)));
  ASSERT_EQ(1, gh._numUnnamedAreas);
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpUnnamedAreaRelations();

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr("... done with looking at 4 areas\n"
                           "                           2 intersection checks "
                           "performed, 2 skipped by DAG\n"
                           "                           intersect: 2 yes: 2\n"
                           "                           3 contains checks "
                           "performed, 1 skipped by DAG\n"
                           "                           contains: 3 contains "
                           "envelope: 1 yes: 1\n"));
  ASSERT_EQ(
      "osmway:11 osm2rdf:intersects_nonarea osmrel:15 .\n"
      "osmway:13 osm2rdf:intersects_nonarea osmrel:15 .\n"
      "osmway:12 osm2rdf:contains_nonarea osmrel:15 .\n",
      printedData);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpUnnamedAreaRelationsSimpleContainsOnly) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  // Unnamed area not from any way.
  osmium::builder::add_area(osmiumBuffer5, osmium::builder::attr::_id(31),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numUnnamedAreas);
  gh.area(osm2rdf::osm::Area(osmiumBuffer5.get<osmium::Area>(0)));
  ASSERT_EQ(1, gh._numUnnamedAreas);
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpUnnamedAreaRelations();

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr("... done with looking at 3 areas\n"
                           "                           1 intersection checks "
                           "performed, 2 skipped by DAG\n"
                           "                           intersect: 1 yes: 1\n"
                           "                           1 contains checks "
                           "performed, 2 skipped by DAG\n"
                           "                           contains: 1 contains "
                           "envelope: 1 yes: 1\n"));
  ASSERT_EQ(
      "osmway:11 osm2rdf:intersects_nonarea osmrel:15 .\n"
      "osmway:11 osm2rdf:contains_nonarea osmrel:15 .\n",
      printedData);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, noNodeGeometricRelations) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noNodeGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpNodeRelations();

  output.flush();
  output.close();

  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "Skipping contains relation for nodes ... disabled"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNodeRelationsEmpty1) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpNodeRelations();

  output.flush();
  output.close();

  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "Skipping contains relation for nodes ... no nodes"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNodeRelationsEmpty2) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpNodeRelations();

  output.flush();
  output.close();

  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "Skipping contains relation for nodes ... no nodes"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNodeRelationsSimpleIntersects) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  // Intersecting 12 and 13 -> 13 in 12 -> only in 13.
  osmium::builder::add_node(
      osmiumBuffer5, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(40.0, 7.55)),
      osmium::builder::attr::_tag("foo", "bar"));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numNodes);
  gh.node(osm2rdf::osm::Node(osmiumBuffer5.get<osmium::Node>(0)));
  ASSERT_EQ(1, gh._numNodes);
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  const auto nd = gh.dumpNodeRelations();
  ASSERT_EQ(1, nd.size());

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "... done with looking at 3 areas, 2 skipped by DAG\n"
                  "                           1 checks performed\n"
                  "                           contains: 1 yes: 1\n"));
  ASSERT_EQ(
      "osmway:13 osm2rdf:intersects_nonarea osmnode:42 .\n"
      "osmway:13 osm2rdf:contains_nonarea osmnode:42 .\n",
      printedData);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpNodeRelationsSimpleContains) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  // Contained in 11.
  osmium::builder::add_node(
      osmiumBuffer5, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(48.05, 7.56)),
      osmium::builder::attr::_tag("foo", "bar"));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numNodes);
  gh.node(osm2rdf::osm::Node(osmiumBuffer5.get<osmium::Node>(0)));
  ASSERT_EQ(1, gh._numNodes);
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  const auto nd = gh.dumpNodeRelations();
  ASSERT_EQ(1, nd.size());

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "... done with looking at 3 areas, 2 skipped by DAG\n"
                  "                           1 checks performed\n"
                  "                           contains: 1 yes: 1\n"));
  ASSERT_EQ(
      "osmway:11 osm2rdf:intersects_nonarea osmnode:42 .\n"
      "osmway:11 osm2rdf:contains_nonarea osmnode:42 .\n",
      printedData);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, noWayGeometricRelations) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noWayGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpWayRelations(osm2rdf::osm::NodesContainedInAreasData{});

  output.flush();
  output.close();

  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr("Skipping contains relation for ways ... disabled"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpWayRelationsEmpty1) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpWayRelations(osm2rdf::osm::NodesContainedInAreasData{});

  output.flush();
  output.close();

  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr("Skipping contains relation for ways ... no ways"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpWayRelationsEmpty2) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpWayRelations(osm2rdf::osm::NodesContainedInAreasData{});

  output.flush();
  output.close();

  ASSERT_THAT(
      cerrBuffer.str(),
      ::testing::HasSubstr("Skipping contains relation for ways ... no ways"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpWayRelationsSimpleIntersects) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {40.1, 7.51}},
                               {2, {48.1, 7.61}},
                               {3, {48.2, 7.61}},
                               {4, {48.2, 7.71}},
                           }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numWays);
  gh.way(osm2rdf::osm::Way(osmiumBuffer5.get<osmium::Way>(0)));
  ASSERT_EQ(1, gh._numWays);
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpWayRelations(osm2rdf::osm::NodesContainedInAreasData{});

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "... done with looking at 4 areas\n"
                  "                           2 intersection checks performed, "
                  "2 skipped by DAG, 0 skipped by NodeInfo\n"
                  "                           intersect: 2 yes: 2\n"
                  "                           3 contains checks performed, 1 "
                  "skipped by DAG\n"
                  "                           contains: 3 contains envelope: 1 "
                  "yes: 1\n"));
  ASSERT_EQ(
      "osmway:11 osm2rdf:intersects_nonarea osmway:42 .\n"
      "osmway:13 osm2rdf:intersects_nonarea osmway:42 .\n"
      "osmway:12 osm2rdf:contains_nonarea osmway:42 .\n",
      printedData);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpWayRelationsSimpleContains) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.02, 7.52}},
                               {2, {48.04, 7.53}},
                               {3, {48.06, 7.59}},
                               {4, {48.08, 7.55}},
                           }));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numWays);
  gh.way(osm2rdf::osm::Way(osmiumBuffer5.get<osmium::Way>(0)));
  ASSERT_EQ(1, gh._numWays);
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  gh.dumpWayRelations(osm2rdf::osm::NodesContainedInAreasData{});

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  ".. done with looking at 3 areas\n"
                  "                           1 intersection checks performed, "
                  "2 skipped by DAG, 0 skipped by NodeInfo\n"
                  "                           intersect: 1 yes: 1\n"
                  "                           1 contains checks performed, 2 "
                  "skipped by DAG\n"
                  "                           contains: 1 contains envelope: 1 "
                  "yes: 1\n"));
  ASSERT_EQ(
      "osmway:11 osm2rdf:intersects_nonarea osmway:42 .\n"
      "osmway:11 osm2rdf:contains_nonarea osmway:42 .\n",
      printedData);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpWayRelationsSimpleIntersectsWithNodeInfo) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer6{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer7{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {40.1, 7.51}},
                               {2, {48.1, 7.61}},
                               {3, {48.2, 7.61}},
                               {4, {48.2, 7.71}},
                           }));
  osmium::builder::add_node(
      osmiumBuffer6, osmium::builder::attr::_id(1),
      osmium::builder::attr::_location(osmium::Location(40.1, 7.51)),
      osmium::builder::attr::_tag("foo", "bar"));
  osmium::builder::add_node(
      osmiumBuffer7, osmium::builder::attr::_id(2),
      osmium::builder::attr::_location(osmium::Location(48.1, 7.61)),
      osmium::builder::attr::_tag("foo", "bar"));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.way(osm2rdf::osm::Way(osmiumBuffer5.get<osmium::Way>(0)));
  gh.node(osm2rdf::osm::Node(osmiumBuffer6.get<osmium::Node>(0)));
  gh.node(osm2rdf::osm::Node(osmiumBuffer7.get<osmium::Node>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  const auto nd = gh.dumpNodeRelations();
  ASSERT_EQ(2, nd.size());
  gh.dumpWayRelations(nd);

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "... done with looking at 4 areas\n"
                  "                           0 intersection checks performed, "
                  "2 skipped by DAG, 2 skipped by NodeInfo\n"
                  "                           intersect: 0 yes: 0\n"
                  "                           3 contains checks performed, 1 "
                  "skipped by DAG\n"
                  "                           contains: 3 contains envelope: 1 "
                  "yes: 1\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:13 osm2rdf:intersects_nonarea osmnode:1 .\n"
                           "osmway:13 osm2rdf:contains_nonarea osmnode:1 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:11 osm2rdf:intersects_nonarea osmnode:2 .\n"
                           "osmway:11 osm2rdf:contains_nonarea osmnode:2 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:11 osm2rdf:intersects_nonarea osmway:42 .\n"
                           "osmway:13 osm2rdf:intersects_nonarea osmway:42 .\n"
                           "osmway:12 osm2rdf:contains_nonarea osmway:42 .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, dumpWayRelationsSimpleContainsWithNodeInfo) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium objects
  /*
           28 (14)
             |
           24 (12)
            /  \
     22 (11)    26 (13)
   */
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer4{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer6{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(22),
                            osmium::builder::attr::_tag("name", "22"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(24),
                            osmium::builder::attr::_tag("name", "24"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.00}},
                                {2, {40.0, 8.00}},
                                {3, {50.0, 8.00}},
                                {4, {50.0, 7.00}},
                                {1, {40.0, 7.00}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(26),
                            osmium::builder::attr::_tag("name", "26"),
                            osmium::builder::attr::_outer_ring({
                                {1, {40.0, 7.51}},
                                {2, {40.0, 7.61}},
                                {3, {40.1, 7.61}},
                                {4, {40.1, 7.51}},
                                {1, {40.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer4, osmium::builder::attr::_id(28),
                            osmium::builder::attr::_tag("name", "28"),
                            osmium::builder::attr::_outer_ring({
                                {1, {20.0, 0.51}},
                                {2, {20.0, 10.61}},
                                {3, {50.1, 10.61}},
                                {4, {50.1, 0.51}},
                                {1, {20.0, 0.51}},
                            }));
  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.02, 7.52}},
                               {2, {48.04, 7.53}},
                               {3, {48.06, 7.59}},
                               {4, {48.08, 7.55}},
                           }));
  osmium::builder::add_node(
      osmiumBuffer6, osmium::builder::attr::_id(2),
      osmium::builder::attr::_location(osmium::Location(48.04, 7.53)),
      osmium::builder::attr::_tag("foo", "bar"));

  // Create osm2rdf object from osmium object
  gh.area(osm2rdf::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2rdf::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  ASSERT_EQ(0, gh._numWays);
  gh.way(osm2rdf::osm::Way(osmiumBuffer5.get<osmium::Way>(0)));
  ASSERT_EQ(1, gh._numWays);
  gh.node(osm2rdf::osm::Node(osmiumBuffer6.get<osmium::Node>(0)));
  gh.closeExternalStorage();
  gh.prepareRTree();
  gh.prepareDAG();

  const auto nd = gh.dumpNodeRelations();
  ASSERT_EQ(1, nd.size());
  gh.dumpWayRelations(nd);

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(cerrBuffer.str(),
              ::testing::HasSubstr(
                  "... done with looking at 3 areas\n"
                  "                           0 intersection checks performed, "
                  "2 skipped by DAG, 1 skipped by NodeInfo\n"
                  "                           intersect: 0 yes: 0\n"
                  "                           1 contains checks performed, 2 "
                  "skipped by DAG\n"
                  "                           contains: 1 contains envelope: 1 "
                  "yes: 1\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:11 osm2rdf:intersects_nonarea osmnode:2 .\n"
                           "osmway:11 osm2rdf:contains_nonarea osmnode:2 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:11 osm2rdf:intersects_nonarea osmway:42 .\n"
                           "osmway:11 osm2rdf:contains_nonarea osmway:42 .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, statisticLine) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, nullptr};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  ASSERT_EQ(
      "{"
      "\"function\":\"\","
      "\"part\":\"\","
      "\"check\":\"\","
      "\"outer_id\":0,"
      "\"outer_type\":\"\","
      "\"inner_id\":0,"
      "\"inner_type\":\"\","
      "\"duration_ns\":0,"
      "\"result\":false"
      "},\n",
      gh.statisticLine("", "", "", 0, "", 0, "",
                       std::chrono::nanoseconds::zero(), false));

  ASSERT_EQ(
      "{"
      "\"function\":\"f\","
      "\"part\":\"p\","
      "\"check\":\"c\","
      "\"outer_id\":1,"
      "\"outer_type\":\"to\","
      "\"inner_id\":2,"
      "\"inner_type\":\"ti\","
      "\"duration_ns\":0,"
      "\"result\":true"
      "},\n",
      gh.statisticLine("f", "p", "c", 1, "to", 2, "ti",
                       std::chrono::nanoseconds::zero(), true));
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, simplifyGeometryArea) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "simplifyGeometryArea-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "simplifyGeometryArea-cache");
  config.simplifyGeometries = 1;
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  const int areaId = 42;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(buffer, osmium::builder::attr::_id(areaId),
                            osmium::builder::attr::_tag("name", "Name"),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {5, {48.0, 7.56}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{buffer.get<osmium::Area>(0)};
  ASSERT_TRUE(src.fromWay());

  ASSERT_EQ(0, gh._spatialStorageArea.size());
  gh.area(src);
  ASSERT_EQ(1, gh._spatialStorageArea.size());

  // Compare stored area with original
  const auto& dst = gh._spatialStorageArea[0];
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() != std::get<2>(dst));
  // Access rings
  const auto srcRings = src.geom();
  const auto dstRings = std::get<2>(dst);
  ASSERT_EQ(1, srcRings.size());
  ASSERT_EQ(1, dstRings.size());
  const auto srcRing = srcRings[0];
  const auto dstRing = dstRings[0];
  ASSERT_EQ(6, srcRing.outer().size());
  ASSERT_EQ(5, dstRing.outer().size());

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_GeometryHandler, simplifyGeometryWay) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_GeometryHandler",
                                     "simplifyGeometryWay-output");
  config.cache = config.getTempPath("TEST_OSM_GeometryHandler",
                                    "simplifyGeometryWay-cache");
  config.simplifyGeometries = 100;
  std::filesystem::create_directories(config.output);
  std::filesystem::create_directories(config.cache);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::GeometryHandler gh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {3, {48.0, 7.52}},
                               {2, {48.1, 7.61}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way src{buffer.get<osmium::Way>(0)};

  ASSERT_EQ(0, gh._numWays);
  gh.way(src);
  ASSERT_EQ(1, gh._numWays);

  // Read area from dump and compare
  osm2rdf::osm::SpatialWayValue dst;

  gh.closeExternalStorage();
  std::ifstream ifs(config.getTempPath("spatial", "ways"), std::ios::binary);
  boost::archive::binary_iarchive ia(ifs);
  ia >> dst;
  ifs.close();

  // Compare stored area with original
  ASSERT_TRUE(src.envelope() == std::get<0>(dst));
  ASSERT_TRUE(src.id() == std::get<1>(dst));
  ASSERT_TRUE(src.geom() != std::get<2>(dst));
  // Access rings
  const auto srcWay = src.geom();
  const auto dstWay = std::get<2>(dst);
  ASSERT_EQ(3, srcWay.size());
  ASSERT_EQ(2, dstWay.size());

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.cache);
  std::filesystem::remove_all(config.output);
}

}  // namespace osm2rdf::osm