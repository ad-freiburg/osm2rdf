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

#include "osm2rdf/osm/FactHandler.h"

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_FactHandler, constructor) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_OSM_DumpHandler", "constructor");
  std::filesystem::create_directories(config.output);
  osm2rdf::util::Output output{config, config.output};
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Assure that no file is written during construction.
  ASSERT_EQ(0, std::distance(std::filesystem::directory_iterator(config.output),
                             std::filesystem::directory_iterator()));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.output);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaFromWay) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }),
                            osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:21 geo:hasGeometry osm2rdfgeom:osmway_21 "
      ".\nosm2rdfgeom:osmway_21 geo:asWKT \"POLYGON((48 7.5,48 7.6,48.1 "
      "7.6,48.1 7.5,48 7.5))\"^^geo:wktLiteral .\nosmway:21 "
      "osm2rdfgeom:convex_hull \"POLYGON((48.1 7.5,48.1 7.6,48 7.6,48 7.5,48.1 "
      "7.5))\"^^geo:wktLiteral .\nosmway:21 osm2rdfgeom:envelope \"POLYGON((48 "
      "7.5,48.1 7.5,48.1 7.6,48 7.6,48 7.5))\"^^geo:wktLiteral .\nosmway:21 "
      "osm2rdfgeom:obb \"POLYGON((48.1 7.5,48.1 7.6,48 7.6,48 7.5,48.1 "
      "7.5))\"^^geo:wktLiteral .\nosmway:21 osm2rdf:area "
      "\"122568687.4654\"^^xsd:double "
      ".\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaFromRelation) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(21),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }),
                            osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmrel:10 geo:hasGeometry osm2rdfgeom:osmrel_10 "
      ".\nosm2rdfgeom:osmrel_10 geo:asWKT \"POLYGON((48 7.5,48 "
      "7.6,48.1 7.6,48.1 7.5,48 7.5))\"^^geo:wktLiteral .\nosmrel:10 "
      "osm2rdfgeom:convex_hull \"POLYGON((48.1 7.5,48.1 7.6,48 7.6,48 7.5,48.1 "
      "7.5))\"^^geo:wktLiteral .\nosmrel:10 osm2rdfgeom:envelope \"POLYGON((48 "
      "7.5,48.1 7.5,48.1 7.6,48 7.6,48 7.5))\"^^geo:wktLiteral .\nosmrel:10 "
      "osm2rdfgeom:obb \"POLYGON((48.1 7.5,48.1 7.6,48 7.6,48 7.5,48.1 "
      "7.5))\"^^geo:wktLiteral .\nosmrel:10 osm2rdf:area "
      "\"122568687.4654\"^^xsd:double "
      ".\nosmrel:10 osm2rdf:hasCompleteGeometry \"true\"^^xsd:boolean .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, node) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  dh.node(osmiumBuffer.get<osmium::Node>(0));
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\nosmnode:42 osmmeta:timestamp "
      "\"1970-01-01T00:00:00\"^^xsd:dateTime .\nosmnode:42 osmmeta:version "
      "\"0\"^^xsd:integer .\nosmnode:42 osmkey:city \"Freiburg\" .\nosmnode:42 "
      "osm2rdf:facts \"1\"^^xsd:integer .\nosmnode:42 geo:hasGeometry "
      "osm2rdfgeom:osmnode_42 .\nosm2rdfgeom:osmnode_42 geo:asWKT "
      "\"POINT(7.5 48)\"^^geo:wktLiteral .\nosmnode:42 osm2rdfgeom:obb "
      "\"POLYGON((7.5 48))\"^^geo:wktLiteral .\nosmnode:42 "
      "osm2rdfgeom:convex_hull \"POLYGON((7.5 48))\"^^geo:wktLiteral "
      ".\nosmnode:42 osm2rdfgeom:envelope \"POLYGON((7.5 48,7.5 48,7.5 48,7.5 "
      "48,7.5 48))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relation) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_member(osmium::item_type::relation, 1, "foo"),
      osmium::builder::attr::_member(osmium::item_type::undefined, 1, "bar"),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{osmiumBuffer.get<osmium::Relation>(0)};

  dh.relation(r);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmrel:42 rdf:type osm:relation .\n"
      "osmrel:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmrel:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osmrel:member_id osmnode:1 .\n"
      "_:0_0 osmrel:member_role \"label\" .\n"
      "_:0_0 osmrel:member_pos \"0\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_1 .\n"
      "_:0_1 osmrel:member_id osmway:1 .\n"
      "_:0_1 osmrel:member_role \"outer\" .\n"
      "_:0_1 osmrel:member_pos \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_2 .\n"
      "_:0_2 osmrel:member_id osmrel:1 .\n"
      "_:0_2 osmrel:member_role \"foo\" .\n"
      "_:0_2 osmrel:member_pos \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationHandler) {
  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

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
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer6{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer1, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::way, 55, "test"),
      osmium::builder::attr::_member(osmium::item_type::way, 56, "test2"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_node(
      osmiumBuffer2, osmium::builder::attr::_id(1),
      osmium::builder::attr::_location(osmium::Location(7.52, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer3, osmium::builder::attr::_id(2),
      osmium::builder::attr::_location(osmium::Location(7.61, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer4, osmium::builder::attr::_id(23),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // 64 bit IDs
  osmium::builder::add_node(
      osmiumBuffer3, osmium::builder::attr::_id(17179869184),
      osmium::builder::attr::_location(osmium::Location(7.61, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer4, osmium::builder::attr::_id(9223372036854775807),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(55),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.52}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_way(osmiumBuffer6, osmium::builder::attr::_id(56),
                           osmium::builder::attr::_nodes({
                               {17179869184, {48.0, 7.52}},
                               {9223372036854775807, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  RelationHandler rh = RelationHandler(config);
  rh.relation(osmiumBuffer1.get<osmium::Relation>(0));
  rh.prepare_for_lookup();
  rh.way(osmiumBuffer5.get<osmium::Way>(0));
  rh.way(osmiumBuffer6.get<osmium::Way>(0));

  auto a = std::vector<uint64_t>{1, 2};
  auto b = rh.get_noderefs_of_way(55);

  ASSERT_EQ(a, b);

  auto c = std::vector<uint64_t>{17179869184, 9223372036854775807};
  auto d = rh.get_noderefs_of_way(56);

  ASSERT_EQ(c, d);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationWithGeometry) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

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
  osmium::memory::Buffer osmiumBuffer5{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer1, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 23, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 55, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_node(
      osmiumBuffer2, osmium::builder::attr::_id(1),
      osmium::builder::attr::_location(osmium::Location(7.52, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer3, osmium::builder::attr::_id(2),
      osmium::builder::attr::_location(osmium::Location(7.61, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer4, osmium::builder::attr::_id(23),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(55),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.52}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  RelationHandler rh = RelationHandler(config);
  LocationHandler* lh = LocationHandler::create(config, 0, 0);
  // Create osm2rdf object from osmium object
  osm2rdf::osm::Relation r{osmiumBuffer1.get<osmium::Relation>(0)};
  rh.relation(osmiumBuffer1.get<osmium::Relation>(0));
  // Fill location and relation handler with data.
  lh->node(osmiumBuffer2.get<osmium::Node>(0));
  lh->node(osmiumBuffer3.get<osmium::Node>(0));
  lh->node(osmiumBuffer4.get<osmium::Node>(0));
  rh.prepare_for_lookup();
  rh.setLocationHandler(lh);
  rh.way(osmiumBuffer5.get<osmium::Way>(0));

  r.buildGeometry(rh);

  dh.relation(r);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmrel:42 rdf:type osm:relation .\n"
      "osmrel:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmrel:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\nosmrel:42 "
      "osmrel:member _:0_0 .\n_:0_0 osmrel:member_id osmnode:23 .\n_:0_0 "
      "osmrel:member_role \"label\" .\n_:0_0 osmrel:member_pos "
      "\"0\"^^xsd:integer .\nosmrel:42 osmrel:member _:0_1 .\n_:0_1 "
      "osmrel:member_id osmway:55 .\n_:0_1 osmrel:member_role \"outer\" "
      ".\n_:0_1 osmrel:member_pos \"1\"^^xsd:integer .\nosmrel:42 "
      "geo:hasGeometry osm2rdfgeom:osmrel_42 "
      ".\nosm2rdfgeom:osmrel_42 geo:asWKT \"GEOMETRYCOLLECTION(POINT(7.5 "
      "48),LINESTRING(7.5 48,7.6 48))\"^^geo:wktLiteral .\nosmrel:42 "
      "osm2rdfgeom:convex_hull \"POLYGON((7.5 48,7.6 48,7.5 "
      "48))\"^^geo:wktLiteral .\nosmrel:42 osm2rdfgeom:envelope \"POLYGON((7.5 "
      "48,7.6 48,7.6 48,7.5 48,7.5 48))\"^^geo:wktLiteral .\nosmrel:42 "
      "osm2rdfgeom:obb \"POLYGON((7.5 48,7.5 48,7.6 48,7.6 48,7.5 "
      "48))\"^^geo:wktLiteral .\nosmrel:42 osm2rdf:hasCompleteGeometry "
      "\"true\"^^xsd:boolean "
      ".\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, way) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addMemberTriples = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmway:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry osm2rdfgeom:osmway_42 .\n"
      "osm2rdfgeom:osmway_42 geo:asWKT \"LINESTRING(48 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:convex_hull \"POLYGON((48 7.5,48.1 7.6,48 "
      "7.5))\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:envelope \"POLYGON((48 7.5,48.1 7.5,48.1 7.6,48 "
      "7.6,48 7.5))\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:obb \"POLYGON((48.1 7.6,48.1 7.6,48 7.5,48 "
      "7.5,48.1 7.6))\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdf:length \"15674.68\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayNodeOrder) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addMemberTriples = true;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmway:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\nosmway:42 "
      "osmway:member _:0_0 .\n_:0_0 osmway:member_id osmnode:1 .\n_:0_0 "
      "osmway:member_pos \"0\"^^xsd:integer .\nosmway:42 osmway:member _:0_1 "
      ".\n_:0_1 osmway:member_id osmnode:2 .\n_:0_1 osmway:member_pos "
      "\"1\"^^xsd:integer .\nosmway:42 geo:hasGeometry osm2rdfgeom:osmway_42 "
      ".\nosm2rdfgeom:osmway_42 geo:asWKT \"LINESTRING(48 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\nosmway:42 osm2rdfgeom:convex_hull "
      "\"POLYGON((48 7.5,48.1 7.6,48 7.5))\"^^geo:wktLiteral .\nosmway:42 "
      "osm2rdfgeom:envelope \"POLYGON((48 7.5,48.1 7.5,48.1 7.6,48 7.6,48 "
      "7.5))\"^^geo:wktLiteral .\nosmway:42 osm2rdfgeom:obb \"POLYGON((48.1 "
      "7.6,48.1 7.6,48 7.5,48 7.5,48.1 7.6))\"^^geo:wktLiteral .\nosmway:42 "
      "osm2rdf:length \"15674.68\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayNodeSpatialMetadataShortWay) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addMemberTriples = true;
  config.addWayNodeSpatialMetadata = true;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmway:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\nosmway:42 "
      "osmway:member _:0_0 .\n_:0_0 osmway:member_id osmnode:1 .\n_:0_0 "
      "osmway:member_pos \"0\"^^xsd:integer .\nosmway:42 osmway:member _:0_1 "
      ".\n_:0_1 osmway:member_id osmnode:2 .\n_:0_1 osmway:member_pos "
      "\"1\"^^xsd:integer .\n_:0_0 osmway:next_node osmnode:2 .\n_:0_0 "
      "osmway:next_node_distance \"15657.137001\"^^xsd:decimal .\nosmway:42 "
      "geo:hasGeometry osm2rdfgeom:osmway_42 .\nosm2rdfgeom:osmway_42 "
      "geo:asWKT "
      "\"LINESTRING(48 7.5,48.1 7.6)\"^^geo:wktLiteral .\nosmway:42 "
      "osm2rdfgeom:convex_hull \"POLYGON((48 7.5,48.1 7.6,48 "
      "7.5))\"^^geo:wktLiteral .\nosmway:42 osm2rdfgeom:envelope \"POLYGON((48 "
      "7.5,48.1 7.5,48.1 7.6,48 7.6,48 7.5))\"^^geo:wktLiteral .\nosmway:42 "
      "osm2rdfgeom:obb \"POLYGON((48.1 7.6,48.1 7.6,48 7.5,48 7.5,48.1 "
      "7.6))\"^^geo:wktLiteral .\nosmway:42 osm2rdf:length "
      "\"15674.68\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayNodeSpatialMetadataLongerWay) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addMemberTriples = true;
  config.addWayNodeSpatialMetadata = true;
  config.addAreaWayLinestrings = true;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                               {4, {48.1, 7.51}},
                               {3, {48.0, 7.51}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmway:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\nosmway:42 "
      "osmway:member _:0_0 .\n_:0_0 osmway:member_id osmnode:1 .\n_:0_0 "
      "osmway:member_pos \"0\"^^xsd:integer .\nosmway:42 osmway:member _:0_1 "
      ".\n_:0_1 osmway:member_id osmnode:2 .\n_:0_1 osmway:member_pos "
      "\"1\"^^xsd:integer .\n_:0_0 osmway:next_node osmnode:2 .\n_:0_0 "
      "osmway:next_node_distance \"15657.137001\"^^xsd:decimal .\nosmway:42 "
      "osmway:member _:0_2 .\n_:0_2 osmway:member_id osmnode:4 .\n_:0_2 "
      "osmway:member_pos \"2\"^^xsd:integer .\n_:0_1 osmway:next_node "
      "osmnode:4 .\n_:0_1 osmway:next_node_distance "
      "\"11119.490351\"^^xsd:decimal .\nosmway:42 osmway:member _:0_3 .\n_:0_3 "
      "osmway:member_id osmnode:3 .\n_:0_3 osmway:member_pos "
      "\"3\"^^xsd:integer "
      ".\n_:0_2 osmway:next_node osmnode:3 .\n_:0_2 osmway:next_node_distance "
      "\"11024.108103\"^^xsd:decimal .\nosmway:42 geo:hasGeometry "
      "osm2rdfgeom:osmway_42 .\nosm2rdfgeom:osmway_42 geo:asWKT "
      "\"LINESTRING(48 "
      "7.5,48.1 "
      "7.6,48.1 7.5,48 7.5)\"^^geo:wktLiteral .\nosmway:42 osm2rdf:length "
      "\"37843.09\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayMetaData) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.addEnvelope = true;
  config.addConvexHull = true;
  config.addObb = true;
  config.addZeroFactNumber = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addWayMetadata = true;
  config.addMemberTriples = false;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmway:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry osm2rdfgeom:osmway_42 .\n"
      "osm2rdfgeom:osmway_42 geo:asWKT \"LINESTRING(48 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:convex_hull \"POLYGON((48 7.5,48.1 7.6,48 "
      "7.5))\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:envelope \"POLYGON((48 7.5,48.1 7.5,48.1 7.6,48 "
      "7.6,48 7.5))\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:obb \"POLYGON((48.1 7.6,48.1 7.6,48 7.5,48 "
      "7.5,48.1 7.6))\"^^geo:wktLiteral .\n"
      "osmway:42 osmway:is_closed \"false\"^^xsd:boolean .\n"
      "osmway:42 osmway:nodeCount \"2\"^^xsd:integer .\n"
      "osmway:42 osmway:uniqueNodeCount \"2\"^^xsd:integer .\n"
      "osmway:42 osm2rdf:length \"15674.68\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeGeometryWay) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  ::util::geo::DLine way;
  way.push_back({0, 0});
  way.push_back({0, 80});
  way.push_back({0, 1000});

  dh.writeGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"LINESTRING(0 0,0 80,0 1000)\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeGeometryWaySimplify1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.simplifyWKT = 2;
  // Simplify all nodes with distance <= 5% of small side (100 * 0.05 = 5)
  config.wktDeviation = 5;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  ::util::geo::DLine way;
  way.push_back({0, 0});
  // Small side is 0 -> remove all nodes except ends.
  way.push_back({0, 80});
  way.push_back({0, 160});
  way.push_back({0, 240});
  way.push_back({0, 500});
  way.push_back({0, 1000});

  dh.writeGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " + "\"LINESTRING(0 0,0 1000)\"" +
                "^^" + osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL +
                " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeGeometryWaySimplify2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.simplifyWKT = 2;
  // Simplify all nodes with distance <= 5% of small side (100 * 0.05 = 5)
  config.wktDeviation = 5;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  ::util::geo::DLine way;
  way.push_back({0, 0});
  way.push_back({0, 80});
  way.push_back({100, 1000});

  dh.writeGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"LINESTRING(0 0,0 80,100 1000)\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeGeometryWaySimplify3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.simplifyWKT = 2;
  // Simplify all nodes with distance <= 80% of small side (100 * 0.8 = 80)
  config.wktDeviation = 80;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  ::util::geo::DLine way;
  way.push_back({0, 0});
  // The node 0,80 will be removed...
  way.push_back({0, 80});
  way.push_back({100, 1000});

  dh.writeGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " + "\"LINESTRING(0 0,100 1000)\"" +
                "^^" + osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL +
                " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeBoxPrecision1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  ::util::geo::DBox box;
  box.setLowerLeft({50, 50});
  box.setUpperRight({200, 200});

  dh.writeBox(subject, predicate, box);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"POLYGON((50 50,200 50,200 200,50 200,50 50))\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeBoxPrecision2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 2;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  ::util::geo::DBox box;
  box.setLowerLeft({50, 50});
  box.setUpperRight({200, 200});

  dh.writeBox(subject, predicate, box);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"POLYGON((50 50,200 50,200 200,50 200,50 50))\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "42";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_nonInteger2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(tagValue, "");
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_nonInteger3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "       ";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(tagValue, "");
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_Integer) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "5";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerPositive) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "+5";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      "5", "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerNegative) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "-5";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      "-5", "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerWS) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "   -5  ";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      "-5", "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerWS2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "+5  ";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      "5", "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_AdminLevel_nonInteger) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "A5";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(tagValue, "");
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_KeyIRI) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "iri";
  const std::string tagValue = "value";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(tagValue, "");
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTag_KeyNotIRI) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "not:Aß%I.R.I\u2000";
  const std::string tagValue = "value";

  const std::string subject = "subject";
  dh.writeTag(subject, tagKey.c_str(), tagValue.c_str());
  const std::string expected = subject +
                               " osm:tag _:0_0 .\n"
                               "_:0_0 osmkey:key \"" +
                               tagKey +
                               "\" .\n"
                               "_:0_0 osmkey:value \"" +
                               tagValue + "\" .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagList) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tag1Key = "admin_level";
  const std::string tag1Value = "42";
  const std::string tag2Key = "iri";
  const std::string tag2Value = "value";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tag1Key);
  const std::string object1 = writer.generateLiteral(
      tag1Value, "^^" + osm2rdf::ttl::constants::IRI__XSD__INTEGER);
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tag2Key);
  const std::string object2 = writer.generateLiteral(tag2Value, "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tag1Key, tag1Value),
      osmium::builder::attr::_tag(tag2Key, tag2Value));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListRefSingle) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "ref";
  const std::string tagValue = "B 3";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListRefDouble) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.semicolonTagKeys.insert("ref");

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "ref";
  const std::string tagValue = "B 3;B 294";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral("B 3", "");
  const std::string object2 = writer.generateLiteral("B 294", "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListRefMultiple) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.semicolonTagKeys.insert("ref");

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "ref";
  const std::string tagValue = "B 3;B 294;K 4917";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral("B 3", "");
  const std::string object2 = writer.generateLiteral("B 294", "");
  const std::string object3 = writer.generateLiteral("K 4917", "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate +
                                                " " + object2 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate +
                                                " " + object3 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListWikidata) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "wikidata";
  const std::string tagValue = "  Q42  ";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, "Q42");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListWikidataMultiple) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "wikidata";
  const std::string tagValue = "Q42;Q1337";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, "Q42");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListWikipediaWithLang) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string value = "Freiburg_im_Breisgau";
  const std::string tagKey = "wikipedia";
  const std::string tagValue = "de:" + value;

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = "<https://de.wikipedia.org/wiki/" + value + ">";

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListWikipediaWithoutLang) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "wikipedia";
  const std::string tagValue = "Freiburg_im_Breisgau";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 =
      "<https://www.wikipedia.org/wiki/" + tagValue + ">";

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListSkipWikiLinks) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.skipWikiLinks = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tag1Key = "wikidata";
  const std::string tag1Value = "  Q42  ";
  const std::string tag2Key = "wikipedia";
  const std::string tag2Value = "de:Freiburg_im_Breisgau";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tag1Key);
  const std::string object1 = writer.generateLiteral(tag1Value, "");
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tag2Key);
  const std::string object2 = writer.generateLiteral(tag2Value, "");
  const std::string predicate3 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tag1Key);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tag1Key, tag1Value),
      osmium::builder::attr::_tag(tag2Key, tag2Value));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));
  ASSERT_THAT(printedData, ::testing::Not(::testing::HasSubstr(predicate3)));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateInvalid) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "lorem";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::Not(::testing::HasSubstr("ear")));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateInvalid2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-lo-r-em-";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::Not(::testing::HasSubstr("ear")));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateInvalid3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-1111-22-33-44";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::Not(::testing::HasSubstr("ear")));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYear1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "11";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      "0011", "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYear2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-11";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      "-0011", "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYear3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "1111";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYear4) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-1111";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonth1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "11-1";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      "0011-01", "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR_MONTH);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonth2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-11-1";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      "-0011-01", "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR_MONTH);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonth3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "1111-11";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR_MONTH);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonth4) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-1111-11";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR_MONTH);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonth5) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-1111-13";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__YEAR_MONTH);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData,
              ::testing::Not(::testing::HasSubstr(subject + " " + predicate2 +
                                                  " " + object2 + " .\n")));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "11-1-1";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      "0011-01-01", "^^" + osm2rdf::ttl::constants::IRI__XSD__DATE);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-11-1-1";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      "-0011-01-01", "^^" + osm2rdf::ttl::constants::IRI__XSD__DATE);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "1111-11-11";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__DATE);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay4) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-1111-11-11";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__DATE);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate2 +
                                                " " + object2 + " .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay5) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.addCentroid = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "start_date";
  const std::string tagValue = "-1111-11-32";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral(tagValue, "");
  const std::string predicate2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG, tagKey);
  const std::string object2 = writer.generateLiteral(
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD__DATE);

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag(tagKey, tagValue));

  dh.writeTagList(subject, osmiumBuffer.get<osmium::Node>(0).tags());
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));
  ASSERT_THAT(printedData,
              ::testing::Not(::testing::HasSubstr(subject + " " + predicate2 +
                                                  " " + object2 + " .\n")));

  // Cleanup
  std::cout.rdbuf(sbuf);
}
}  // namespace osm2rdf::osm
