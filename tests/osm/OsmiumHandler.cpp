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

#include "osm2rdf/osm/OsmiumHandler.h"

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"
#include "osmium/io/bzip2_compression.hpp"
#include "osmium/io/detail/o5m_input_format.hpp"
#include "osmium/io/detail/pbf_input_format.hpp"
#include "osmium/io/detail/xml_input_format.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
template <typename W>
void addOsmiumItems(osm2rdf::osm::OsmiumHandler<W>* oh) {
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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, constructor) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noFacts) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noFacts = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noGeometricRelations) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noAreaFacts) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noAreaFacts = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(0, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noNodeFacts) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noNodeFacts = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(0, oh.nodesDumped());
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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noRelationFacts) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noRelationFacts = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(1, oh.nodesDumped());
  ASSERT_EQ(1, oh.nodeGeometriesHandled());
  ASSERT_EQ(3, oh.relationsSeen());
  ASSERT_EQ(0, oh.relationsDumped());
  ASSERT_EQ(0, oh.relationGeometriesHandled());
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(1, oh.waysDumped());
  ASSERT_EQ(1, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noWayFacts) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noWayFacts = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

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
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(0, oh.waysDumped());
  ASSERT_EQ(1, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noAreaGeometricRelations) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noAreaGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noNodeGeometricRelations) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noNodeGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

  addOsmiumItems(&oh);

  ASSERT_EQ(2, oh.areasSeen());
  ASSERT_EQ(2, oh.areasDumped());
  ASSERT_EQ(2, oh.areaGeometriesHandled());
  ASSERT_EQ(2, oh.nodesSeen());
  ASSERT_EQ(1, oh.nodesDumped());
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

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, noWayGeometricRelations) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.noWayGeometricRelations = true;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::OsmiumHandler oh{config, &writer};

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
  ASSERT_EQ(2, oh.waysSeen());
  ASSERT_EQ(1, oh.waysDumped());
  ASSERT_EQ(0, oh.wayGeometriesHandled());

  // Cleanup
  output.close();
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyPBF) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.pbf");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  ASSERT_THROW(osmiumHandler.handle(), osmium::pbf_error);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyOSM) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.osm");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  ASSERT_THROW(osmiumHandler.handle(), osmium::xml_error);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyBzip2OSM) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.osm.bz2");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  ASSERT_THROW(osmiumHandler.handle(), osmium::bzip2_error);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyOPL) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.opl");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyBzip2OPL) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.opl.bz2");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  ASSERT_THROW(osmiumHandler.handle(), osmium::bzip2_error);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyO5M) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.o5m");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  ASSERT_THROW(osmiumHandler.handle(), osmium::o5m_error);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleEmptyBzip2O5M) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "empty.o5m.bz2");
  std::ofstream inputFile(config.input);

  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  ASSERT_THROW(osmiumHandler.handle(), osmium::bzip2_error);

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleSingleNode) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "singleNode.osm");
  std::ofstream inputFile(config.input);
  // Copied from
  // https://wiki.openstreetmap.org/w/index.php?title=OSM_XML&oldid=2081001
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\" generator=\"CGImap 0.0.2\">\n"
            << "<node id=\"298884269\" lat=\"54.0901746\" lon=\"12.2482632\" "
               "user=\"SvenHRO\" uid=\"46882\" visible=\"true\" version=\"1\" "
               "changeset=\"676636\" timestamp=\"2008-09-21T21:37:45Z\"/>"
            << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:1 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:0 dumped: 0 geometry: 0\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(OSM_OsmiumHandler, handleOSMWikiExample) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("OSM_OsmiumHandler", "osmWikiExample.osm");
  std::ofstream inputFile(config.input);
  // Based on
  // https://wiki.openstreetmap.org/w/index.php?title=OSM_XML&oldid=2081001
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\" generator=\"CGImap 0.0.2\">\n";
  inputFile << " <node id=\"298884269\" lat=\"54.0901746\" lon=\"12.2482632\" "
               "visible=\"true\" version=\"1\"/>\n";
  inputFile << " <node id=\"261728686\" lat=\"54.0906309\" lon=\"12.2441924\" "
               "visible=\"true\" version=\"1\"/>\n";
  inputFile << " <node id=\"1831881213\" version=\"1\" lat=\"54.0900666\" "
               "lon=\"12.2539381\" visible=\"true\">\n"
               "  <tag k=\"name\" v=\"Neu Broderstorf\"/>\n"
               "  <tag k=\"traffic_sign\" v=\"city_limit\"/>\n"
               " </node>\n";
  inputFile << " <node id=\"298884272\" lat=\"54.0901447\" lon=\"12.2516513\" "
               "visible=\"true\" version=\"1\"/>\n";
  inputFile << " <way id=\"26659127\" visible=\"true\" version=\"5\">\n"
               "  <nd ref=\"298884269\"/>\n"
               "  <nd ref=\"261728686\"/>\n"
               "  <nd ref=\"298884272\"/>\n"
               "  <tag k=\"highway\" v=\"unclassified\"/>\n"
               "  <tag k=\"name\" v=\"Pastower Straße\"/>\n"
               " </way>\n";
  inputFile << " <relation id=\"56688\" visible=\"true\" version=\"28\">\n"
               "  <member type=\"node\" ref=\"298884269\" role=\"\"/>\n"
               "  <member type=\"node\" ref=\"261728686\" role=\"\"/>\n"
               "  <member type=\"way\" ref=\"26659127\" role=\"\"/>\n"
               "  <member type=\"node\" ref=\"1831881213\" role=\"\"/>\n"
               "  <tag k=\"name\" v=\"Küstenbus Linie 123\"/>\n"
               "  <tag k=\"network\" v=\"VVW\"/>\n"
               "  <tag k=\"operator\" v=\"Regionalverkehr Küste\"/>\n"
               "  <tag k=\"ref\" v=\"123\"/>\n"
               "  <tag k=\"route\" v=\"bus\"/>\n"
               "  <tag k=\"type\" v=\"route\"/>\n"
               " </relation>";
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:4 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:1 dumped: 1 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

}  // namespace osm2rdf::osm
