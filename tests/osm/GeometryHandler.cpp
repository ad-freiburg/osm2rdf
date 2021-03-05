// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include <omp.h>

#include "boost/archive/binary_iarchive.hpp"
#include "boost/geometry.hpp"
#include "gmock/gmock-matchers.h"
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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2ttl object from osmium object
  gh.area(osm2ttl::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));

  ASSERT_EQ(0, gh._spatialIndex.size());
  gh.prepareRTree();
  ASSERT_EQ(gh._spatialStorageArea.size(), gh._spatialIndex.size());

  std::vector<SpatialAreaValue> queryResult;
  osm2ttl::geometry::Box nodeEnvelope;

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2ttl::geometry::Location(148.05, 7.56);
  nodeEnvelope.max_corner() = osm2ttl::geometry::Location(148.05, 7.56);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));
  ASSERT_EQ(0, queryResult.size());

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2ttl::geometry::Location(45.00, 8.00);
  nodeEnvelope.max_corner() = osm2ttl::geometry::Location(45.00, 8.00);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));

  // 24, 28
  ASSERT_EQ(2, queryResult.size());

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2ttl::geometry::Location(48.05, 7.56);
  nodeEnvelope.max_corner() = osm2ttl::geometry::Location(48.05, 7.56);
  gh._spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));
  // 22, 24, 28
  ASSERT_EQ(3, queryResult.size());

  queryResult.clear();
  nodeEnvelope.min_corner() = osm2ttl::geometry::Location(40.05, 7.56);
  nodeEnvelope.max_corner() = osm2ttl::geometry::Location(40.05, 7.56);
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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2ttl object from osmium object
  gh.area(osm2ttl::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.prepareRTree();

  ASSERT_EQ(0, gh._directedAreaGraph.getNumVertices());
  ASSERT_EQ(0, gh._directedAreaGraph.getNumEdges());
  gh.prepareDAG();
  ASSERT_EQ(4, gh._directedAreaGraph.getNumVertices());
  ASSERT_EQ(3, gh._directedAreaGraph.getNumEdges());

  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(22));
  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(26));
  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{28},
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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2ttl object from osmium object
  gh.area(osm2ttl::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.prepareRTree();
  gh.prepareDAG();

  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(22));
  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(26));
  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{28},
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
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 11)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 11)));

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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::GeometryHandler gh{config, &writer};

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

  // Create osm2ttl object from osmium object
  gh.area(osm2ttl::osm::Area(osmiumBuffer1.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer2.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer3.get<osmium::Area>(0)));
  gh.area(osm2ttl::osm::Area(osmiumBuffer4.get<osmium::Area>(0)));
  gh.prepareRTree();
  gh.prepareDAG();

  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(22));
  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{24},
            gh._directedAreaGraph.getEdges(26));
  ASSERT_EQ(std::vector<osm2ttl::osm::Area::id_t>{28},
            gh._directedAreaGraph.getEdges(24));

  gh.dumpNamedAreaRelations();

  output.flush();
  output.close();

  const std::string printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 14) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 13)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "contains_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 11)));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 12) +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OPENGIS,
                             "intersects_area") +
          " " +
          writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, 11)));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

#endif

}  // namespace osm2ttl::osm