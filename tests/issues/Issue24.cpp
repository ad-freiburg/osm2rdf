// Copyright 2023, University of Freiburg
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

#include "gtest/gtest.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::util {

// ____________________________________________________________________________
TEST(Issue24, areaFromWayHasGeometryAsGeoSPARQL) {
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
  config.addConvexHull = false;
  config.addObb = false;
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
                            }));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:21 geo:hasGeometry osm2rdfgeom:osmway_21 "
      ".\nosm2rdfgeom:osmway_21 geo:asWKT \"POLYGON((48 7.5,48 "
      "7.6,48.1 7.6,48.1 7.5,48 7.5))\"^^geo:wktLiteral "
      ".\nosmway:21 "
      "osm2rdfgeom:envelope \"POLYGON((48 7.5,48.1 7.5,48.1 7.6,48 7.6,48 "
      "7.5))\"^^geo:wktLiteral .\nosmway:21 "
      "osm2rdf:area \"0.01\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(Issue24, areaFromRelationHasGeometryAsGeoSPARQL) {
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
  config.addEnvelope = false;
  config.addConvexHull = false;
  config.addObb = false;
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
                            }));

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
      "osm2rdf:area \"0.01\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(Issue24, nodeHasGeometryAsGeoSPARQL) {
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
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));

  dh.node(osmiumBuffer.get<osmium::Node>(0));
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\nosmnode:42 osmmeta:timestamp "
      "\"1970-01-01T00:00:00\"^^xsd:dateTime .\nosmnode:42 osmmeta:version "
      "\"0\"^^xsd:integer .\nosmnode:42 osm2rdf:facts \"0\"^^xsd:integer "
      ".\nosmnode:42 geo:hasGeometry osm2rdfgeom:osmnode_42 "
      ".\nosm2rdfgeom:osmnode_42 geo:asWKT \"POINT(7.5 48)\"^^geo:wktLiteral "
      ".\nosmnode:42 osm2rdfgeom:obb \"POLYGON((7.5 48))\"^^geo:wktLiteral "
      ".\nosmnode:42 osm2rdfgeom:convex_hull \"POLYGON((7.5 "
      "48))\"^^geo:wktLiteral .\nosmnode:42 osm2rdfgeom:envelope "
      "\"POLYGON((7.5 48,7.5 48,7.5 48,7.5 48,7.5 48))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(Issue24, relationWithGeometryHasGeometryAsGeoSPARQL) {
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
      osmium::builder::attr::_member(osmium::item_type::way, 55, "outer"));
  osmium::builder::add_node(
      osmiumBuffer2, osmium::builder::attr::_id(1),
      osmium::builder::attr::_location(osmium::Location(7.52, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer3, osmium::builder::attr::_id(2),
      osmium::builder::attr::_location(osmium::Location(7.61, 48.0)));
  osmium::builder::add_node(
      osmiumBuffer4, osmium::builder::attr::_id(23),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));
  osmium::builder::add_way(osmiumBuffer5, osmium::builder::attr::_id(55),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.52}},
                               {2, {48.1, 7.61}},
                           }));

  osm2rdf::osm::RelationHandler rh = osm2rdf::osm::RelationHandler(config);
  osm2rdf::osm::LocationHandler* lh =
      osm2rdf::osm::LocationHandler::create(config, 0, 0);
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
      "osmrel:42 osm2rdf:facts "
      "\"0\"^^xsd:integer .\nosmrel:42 osmrel:member _:0_0 .\n_:0_0 "
      "osmrel:member_id osmnode:23 .\n_:0_0 osmrel:member_role \"label\" "
      ".\n_:0_0 osmrel:member_pos \"0\"^^xsd:integer .\nosmrel:42 "
      "osmrel:member _:0_1 .\n_:0_1 osmrel:member_id osmway:55 .\n_:0_1 "
      "osmrel:member_role \"outer\" .\n_:0_1 osmrel:member_pos "
      "\"1\"^^xsd:integer .\nosmrel:42 geo:hasGeometry "
      "osm2rdfgeom:osmrel_42 .\nosm2rdfgeom:osmrel_42 geo:asWKT "
      "\"GEOMETRYCOLLECTION(POINT(7.5 48),LINESTRING(7.5 48,7.6 "
      "48))\"^^geo:wktLiteral .\nosmrel:42 osm2rdfgeom:convex_hull "
      "\"POLYGON((7.5 48,7.6 48,7.5 48))\"^^geo:wktLiteral .\nosmrel:42 "
      "osm2rdfgeom:envelope \"POLYGON((7.5 48,7.6 48,7.6 48,7.5 48,7.5 "
      "48))\"^^geo:wktLiteral .\nosmrel:42 osm2rdfgeom:obb \"POLYGON((7.5 48,"
      "7.5 48,7.6 48,7.6 48,7.5 48))\"^^geo:wktLiteral .\nosmrel:42 "
      "osm2rdf:hasCompleteGeometry \"true\"^^xsd:boolean .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(Issue24, wayHasGeometryAsGeoSPARQL) {
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
                           }));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmmeta:timestamp \"1970-01-01T00:00:00\"^^xsd:dateTime .\n"
      "osmway:42 osmmeta:version \"0\"^^xsd:integer .\n"
      "osmway:42 osm2rdf:facts "
      "\"0\"^^xsd:integer .\nosmway:42 geo:hasGeometry osm2rdfgeom:osmway_42 "
      ".\nosm2rdfgeom:osmway_42 geo:asWKT \"LINESTRING(48 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\nosmway:42 osm2rdfgeom:convex_hull "
      "\"POLYGON((48 7.5,48.1 7.6,48 7.5))\"^^geo:wktLiteral .\nosmway:42 "
      "osm2rdfgeom:envelope \"POLYGON((48 7.5,48.1 7.5,48.1 7.6,48 7.6,48 "
      "7.5))\"^^geo:wktLiteral .\nosmway:42 osm2rdfgeom:obb \"POLYGON((48.1 "
      "7.6,48.1 7.6,48 7.5,48 7.5,48.1 7.6))\"^^geo:wktLiteral "
      ".\nosmway:42 osm2rdf:length \"0.141421\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

}  // namespace osm2rdf::util
