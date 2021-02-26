// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/OsmiumHandler.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

template <typename W>
void addOsmiumItems(osm2ttl::osm::OsmiumHandler<W>* oh) {
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};

  // Area
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  oh->area(osmiumBuffer.get<osmium::Area>(0));
  osmiumBuffer.clear();
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }),
                            osmium::builder::attr::_tag("admin_level", "42"));
  oh->area(osmiumBuffer.get<osmium::Area>(0));

  // Node
  osmiumBuffer.clear();
  // Node without tags will always be ignored
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(43),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));
  oh->node(osmiumBuffer.get<osmium::Node>(0));
  osmiumBuffer.clear();
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));
  oh->node(osmiumBuffer.get<osmium::Node>(0));

  // Relation
  osmiumBuffer.clear();
  osmium::builder::add_relation(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"));
  oh->relation(osmiumBuffer.template get<osmium::Relation>(0));
  osmiumBuffer.clear();
  osmium::builder::add_relation(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  oh->relation(osmiumBuffer.template get<osmium::Relation>(0));
  osmiumBuffer.clear();
  osmium::builder::add_relation(
      osmiumBuffer, osmium::builder::attr::_id(44),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"),
      osmium::builder::attr::_tag("admin_level", "42"));
  ;
  oh->relation(osmiumBuffer.template get<osmium::Relation>(0));

  // Way
  osmiumBuffer.clear();
  // Way without tags will always be ignored
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {42.0, 7.51}},
                               {2, {42.1, 7.61}},
                           }));
  oh->way(osmiumBuffer.get<osmium::Way>(0));
  osmiumBuffer.clear();
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));
  oh->way(osmiumBuffer.get<osmium::Way>(0));
}

TEST(OSM_OsmiumHandler, constructor) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
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
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, noDump) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.noFacts = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(0, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(0, oh.nodesDumped());
  ASSERT_EQ(1, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(0, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(0, oh.waysDumped());
  ASSERT_EQ(1, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, noContains) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.noGeometricRelations = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(0, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(1, oh.nodesDumped());
  ASSERT_EQ(0, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(2, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(1, oh.waysDumped());
  ASSERT_EQ(0, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, adminRelationsOnly) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.adminRelationsOnly = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(1, oh.areasDumped());
  ASSERT_EQ(1, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(0, oh.nodesDumped());
  ASSERT_EQ(0, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(1, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(0, oh.waysDumped());
  ASSERT_EQ(0, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, noAreaDump) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.noAreas = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(0, oh.areasSeen());
  ASSERT_EQ(0, oh.areasDumped());
  ASSERT_EQ(0, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(1, oh.nodesDumped());
  ASSERT_EQ(1, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(2, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(1, oh.waysDumped());
  ASSERT_EQ(1, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, noNodeDump) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.noNodes = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(0, oh.nodesSeen());
  ASSERT_EQ(0, oh.nodesDumped());
  ASSERT_EQ(0, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(2, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(1, oh.waysDumped());
  ASSERT_EQ(1, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, noRelationDump) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.noRelations = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(1, oh.nodesDumped());
  ASSERT_EQ(1, oh.nodeGeometriesHandled());
  ASSERT_EQ(0, oh.relationsSeen());
  ASSERT_EQ(0, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(1, oh.waysDumped());
  ASSERT_EQ(1, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

TEST(OSM_OsmiumHandler, noWayDump) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.noWays = true;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(1, oh.nodesDumped());
  ASSERT_EQ(1, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(2, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(0, oh.waysSeen());
  ASSERT_EQ(0, oh.waysDumped());
  ASSERT_EQ(0, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

}  // namespace osm2ttl::osm