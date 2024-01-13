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

#include "boost/version.hpp"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/Node.h"
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;

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
  const osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;

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
  const osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmrel:10 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaAddConvexHull) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addAreaConvexHull = true;
  config.addSortMetadata = false;

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
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm2rdfgeom:convex_hull \"POLYGON((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 "
      "7.5,48.0 7.5))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaAddEnvelope) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addAreaEnvelope = true;
  config.addSortMetadata = false;

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
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm2rdfgeom:envelope \"POLYGON((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 "
      "7.5,48.0 7.5))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaAddOrientedBoundingBox) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addAreaOrientedBoundingBox = true;
  config.addSortMetadata = false;

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
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm2rdfgeom:obb \"POLYGON((48.0 7.6,48.1 7.6,48.1 7.5,48.0 "
      "7.5,48.0 7.6))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaAddSortMetadata) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = true;

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
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm2rdf:area \"0.010000000000\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaAddAreaEnvelopeRatioRectangle) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addAreaEnvelopeRatio = true;

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
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm2rdf:area \"0.010000000000\"^^xsd:double .\n"
      "osmway:21 osm2rdf:area_envelope_ratio \"1.000000\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, areaAddAreaEnvelopeRatioDiamond) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addAreaEnvelopeRatio = true;

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
                                {1, {0.5, 0.0}},
                                {2, {1.0, 0.5}},
                                {3, {0.5, 1.0}},
                                {4, {0.0, 0.5}},
                                {1, {0.5, 0.0}},
                            }),
                            osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((0.5 0.0,0.0 0.5,0.5 1.0,1.0 "
      "0.5,0.5 0.0)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm2rdf:area \"0.500000000000\"^^xsd:double .\n"
      "osmway:21 osm2rdf:area_envelope_ratio \"0.500000\"^^xsd:double .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};

  dh.node(n);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\n"
      "osmnode:42 geo:hasGeometry \"POINT(7.5 48.0)\"^^geo:wktLiteral .\n"
      "osmnode:42 osmkey:city \"Freiburg\" .\n"
      "osmnode:42 osm2rdf:facts \"1\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, nodeAddConvexHull) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addNodeConvexHull = true;

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};

  dh.node(n);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\n"
      "osmnode:42 geo:hasGeometry \"POINT(7.5 48.0)\"^^geo:wktLiteral .\n"
      "osmnode:42 osmkey:city \"Freiburg\" .\n"
      "osmnode:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmnode:42 osm2rdfgeom:convex_hull \"POLYGON((7.5 48.0,7.5 48.0,7.5 "
      "48.0,7.5 "
      "48.0,7.5 48.0))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, nodeAddEnvelope) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addNodeEnvelope = true;

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};

  dh.node(n);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\n"
      "osmnode:42 geo:hasGeometry \"POINT(7.5 48.0)\"^^geo:wktLiteral .\n"
      "osmnode:42 osmkey:city \"Freiburg\" .\n"
      "osmnode:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmnode:42 osm2rdfgeom:envelope \"POLYGON((7.5 48.0,7.5 48.0,7.5 "
      "48.0,7.5 "
      "48.0,7.5 48.0))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, nodeAddOrientedBoundingBox) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addNodeOrientedBoundingBox = true;

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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};

  dh.node(n);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\n"
      "osmnode:42 geo:hasGeometry \"POINT(7.5 48.0)\"^^geo:wktLiteral .\n"
      "osmnode:42 osmkey:city \"Freiburg\" .\n"
      "osmnode:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmnode:42 osm2rdfgeom:obb \"POLYGON((7.5 48.0,7.5 48.0,7.5 48.0,7.5 "
      "48.0,7.5 48.0))\"^^geo:wktLiteral .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:1 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_1 .\n"
      "_:0_1 osm:id osmrel:1 .\n"
      "_:0_1 osm:role \"foo\" .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_2 .\n"
      "_:0_2 osm:id osm:1 .\n"
      "_:0_2 osm:role \"bar\" .\n"
      "_:0_2 osm2rdf:pos \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationAddConvexHull) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addRelationConvexHull = true;

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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:1 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_1 .\n"
      "_:0_1 osm:id osmrel:1 .\n"
      "_:0_1 osm:role \"foo\" .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_2 .\n"
      "_:0_2 osm:id osm:1 .\n"
      "_:0_2 osm:role \"bar\" .\n"
      "_:0_2 osm2rdf:pos \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationAddEnvelop) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addRelationEnvelope = true;

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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:1 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_1 .\n"
      "_:0_1 osm:id osmrel:1 .\n"
      "_:0_1 osm:role \"foo\" .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_2 .\n"
      "_:0_2 osm:id osm:1 .\n"
      "_:0_2 osm:role \"bar\" .\n"
      "_:0_2 osm2rdf:pos \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationAddOrientedBoundingBox) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addRelationOrientedBoundingBox = true;

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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:1 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_1 .\n"
      "_:0_1 osm:id osmrel:1 .\n"
      "_:0_1 osm:role \"foo\" .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_2 .\n"
      "_:0_2 osm:id osm:1 .\n"
      "_:0_2 osm:role \"bar\" .\n"
      "_:0_2 osm2rdf:pos \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

#if BOOST_VERSION >= 107800
// ____________________________________________________________________________
TEST(OSM_FactHandler, relationWithGeometry) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  LocationHandler* lh = LocationHandler::create(config);
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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:23 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 geo:hasGeometry \"GEOMETRYCOLLECTION(POINT(7.5 "
      "48.0),LINESTRING(7.5 48.0,7.6 48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdf:completeGeometry \"yes\" .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationWithGeometryAddConvexHull) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addRelationConvexHull = true;

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
  LocationHandler* lh = LocationHandler::create(config);
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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:23 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 geo:hasGeometry \"GEOMETRYCOLLECTION(POINT(7.5 "
      "48.0),LINESTRING(7.5 48.0,7.6 48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdfgeom:convex_hull \"POLYGON((7.5 48.0,7.6 48.0,7.5 "
      "48.0,7.5 "
      "48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdf:completeGeometry \"yes\" .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationWithGeometryAddEnvelope) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addRelationEnvelope = true;

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
  LocationHandler* lh = LocationHandler::create(config);
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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:23 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 geo:hasGeometry \"GEOMETRYCOLLECTION(POINT(7.5 "
      "48.0),LINESTRING(7.5 48.0,7.6 48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdfgeom:envelope \"POLYGON((7.5 48.0,7.5 48.0,7.6 "
      "48.0,7.6 "
      "48.0,7.5 48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdf:completeGeometry \"yes\" .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, relationWithGeometryAddOrientedBoundingBox) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addRelationOrientedBoundingBox = true;

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
  LocationHandler* lh = LocationHandler::create(config);
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
      "osmrel:42 osmkey:city \"Freiburg\" .\n"
      "osmrel:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmrel:42 osmrel:member _:0_0 .\n"
      "_:0_0 osm:id osmnode:23 .\n"
      "_:0_0 osm:role \"label\" .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmrel:42 geo:hasGeometry \"GEOMETRYCOLLECTION(POINT(7.5 "
      "48.0),LINESTRING(7.5 48.0,7.6 48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdfgeom:obb \"POLYGON((7.6 48.0,7.6 48.0,7.5 48.0,7.5 "
      "48.0,7.6 48.0))\"^^geo:wktLiteral .\n"
      "osmrel:42 osm2rdf:completeGeometry \"yes\" .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}
#endif  // BOOST_VERSION >= 107800

// ____________________________________________________________________________
TEST(OSM_FactHandler, way) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddSortMetadata) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  // osm2rdf:length should be a multiple of sqrt(2)
  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdf:length \"0.141421\"^^xsd:double .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayConvexHull) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayConvexHull = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:convex_hull \"POLYGON((48.0 7.5,48.1 7.6,48.0 "
      "7.5,48.0 7.5))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayEnvelope) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayEnvelope = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:envelope \"POLYGON((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 "
      "7.5,48.0 7.5))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayOrientedBoundingBox) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayOrientedBoundingBox = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm2rdfgeom:obb \"POLYGON((48.1 7.6,48.1 7.6,48.0 7.5,48.0 "
      "7.5,48.1 7.6))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, wayAddWayNodeGeoemtry) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayNodeGeometry = true;
  config.addWayNodeOrder = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_0 .\n"
      "_:0_0 osmway:node osmnode:1 .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmnode:1 rdf:type osm:node .\n"
      "osmnode:1 geo:hasGeometry \"POINT(48.0 7.5)\"^^geo:wktLiteral .\n"
      "osmway:42 osmway:node _:0_1 .\n"
      "_:0_1 osmway:node osmnode:2 .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "osmnode:2 rdf:type osm:node .\n"
      "osmnode:2 geo:hasGeometry \"POINT(48.1 7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayNodeOrder = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_0 .\n"
      "_:0_0 osmway:node osmnode:1 .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_1 .\n"
      "_:0_1 osmway:node osmnode:2 .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayNodeOrder = true;
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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_0 .\n"
      "_:0_0 osmway:node osmnode:1 .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_1 .\n"
      "_:0_1 osmway:node osmnode:2 .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "_:0_0 osmway:next_node osmnode:2 .\n"
      "_:0_0 osmway:next_node_distance \"15657.137001\"^^xsd:decimal .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayNodeOrder = true;
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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_0 .\n"
      "_:0_0 osmway:node osmnode:1 .\n"
      "_:0_0 osm2rdf:pos \"0\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:0_1 .\n"
      "_:0_1 osmway:node osmnode:2 .\n"
      "_:0_1 osm2rdf:pos \"1\"^^xsd:integer .\n"
      "_:0_0 osmway:next_node osmnode:2 .\n"
      "_:0_0 osmway:next_node_distance \"15657.137001\"^^xsd:decimal .\n"
      "osmway:42 osmway:node _:0_2 .\n"
      "_:0_2 osmway:node osmnode:4 .\n"
      "_:0_2 osm2rdf:pos \"2\"^^xsd:integer .\n"
      "_:0_1 osmway:next_node osmnode:4 .\n"
      "_:0_1 osmway:next_node_distance \"11119.490351\"^^xsd:decimal .\n"
      "osmway:42 osmway:node _:0_3 .\n"
      "_:0_3 osmway:node osmnode:3 .\n"
      "_:0_3 osm2rdf:pos \"3\"^^xsd:integer .\n"
      "_:0_2 osmway:next_node osmnode:3 .\n"
      "_:0_2 osmway:next_node_distance \"11024.108103\"^^xsd:decimal .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 7.6,48.1 7.5,48.0 "
      "7.5)\"^^geo:wktLiteral .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addSortMetadata = false;
  config.addWayMetadata = true;

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
  const osm2rdf::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmkey:city \"Freiburg\" .\n"
      "osmway:42 osm2rdf:facts \"1\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osmway:is_closed \"no\" .\n"
      "osmway:42 osmway:nodeCount \"2\"^^xsd:integer .\n"
      "osmway:42 osmway:uniqueNodeCount \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeBoostGeometryWay) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  osm2rdf::geometry::Way way;
  way.push_back(osm2rdf::geometry::Location{0, 0});
  way.push_back(osm2rdf::geometry::Location{0, 80});
  way.push_back(osm2rdf::geometry::Location{0, 1000});

  dh.writeBoostGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"LINESTRING(0.0 0.0,0.0 80.0,0.0 1000.0)\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeBoostGeometryWaySimplify1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  osm2rdf::geometry::Way way;
  way.push_back(osm2rdf::geometry::Location{0, 0});
  // Small side is 0 -> remove all nodes except ends.
  way.push_back(osm2rdf::geometry::Location{0, 80});
  way.push_back(osm2rdf::geometry::Location{0, 160});
  way.push_back(osm2rdf::geometry::Location{0, 240});
  way.push_back(osm2rdf::geometry::Location{0, 500});
  way.push_back(osm2rdf::geometry::Location{0, 1000});

  dh.writeBoostGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"LINESTRING(0.0 0.0,0.0 1000.0)\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeBoostGeometryWaySimplify2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  osm2rdf::geometry::Way way;
  way.push_back(osm2rdf::geometry::Location{0, 0});
  way.push_back(osm2rdf::geometry::Location{0, 80});
  way.push_back(osm2rdf::geometry::Location{100, 1000});

  dh.writeBoostGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"LINESTRING(0.0 0.0,0.0 80.0,100.0 1000.0)\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(OSM_FactHandler, writeBoostGeometryWaySimplify3) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  osm2rdf::geometry::Way way;
  way.push_back(osm2rdf::geometry::Location{0, 0});
  // The node 0,80 will be removed...
  way.push_back(osm2rdf::geometry::Location{0, 80});
  way.push_back(osm2rdf::geometry::Location{100, 1000});

  dh.writeBoostGeometry(subject, predicate, way);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"LINESTRING(0.0 0.0,100.0 1000.0)\"" + "^^" +
                osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL + " .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  osm2rdf::geometry::Box box;
  box.min_corner() = osm2rdf::geometry::Location{50, 50};
  box.max_corner() = osm2rdf::geometry::Location{200, 200};

  dh.writeBox(subject, predicate, box);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"POLYGON((50.0 50.0,50.0 200.0,200.0 200.0,200.0 50.0,50.0 "
                "50.0))\"" +
                "^^" + osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL +
                " .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.wktPrecision = 2;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  osm2rdf::geometry::Box box;
  box.min_corner() = osm2rdf::geometry::Location{50, 50};
  box.max_corner() = osm2rdf::geometry::Location{200, 200};

  dh.writeBox(subject, predicate, box);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"POLYGON((50.00 50.00,50.00 200.00,200.00 200.00,200.00 "
                "50.00,50.00 50.00))\"" +
                "^^" + osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL +
                " .\n",
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      tagValue, "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      "5", "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      "-5", "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      "-5", "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      "5", "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "not:Aß%I.R.I\u2000";
  const std::string tagValue = "value";

  const std::string subject = "subject";
  dh.writeTag(subject, osm2rdf::osm::Tag{tagKey, tagValue});
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
      tag1Value, "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER);
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tag2Key);
  const std::string object2 = writer.generateLiteral(tag2Value, "");

  osm2rdf::osm::TagList tagList;
  tagList[tag1Key] = tag1Value;
  tagList[tag2Key] = tag2Value;

  dh.writeTagList(subject, tagList);
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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

  osm2rdf::osm::TagList tagList;
  tagList[tagKey] = tagValue;

  dh.writeTagList(subject, tagList);
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  ASSERT_THAT(printedData, ::testing::HasSubstr(subject + " " + predicate1 +
                                                " " + object1 + " .\n"));

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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.semicolonTagKeys.insert("ref");

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  osm2rdf::osm::FactHandler dh{config, &writer};

  const std::string tagKey = "ref";
  const std::string tagValue = "B 3;B 294";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object1 = writer.generateLiteral("B 3", "");
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object2 = writer.generateLiteral("B 294", "");

  osm2rdf::osm::TagList tagList;
  tagList[tagKey] = tagValue;

  dh.writeTagList(subject, tagList);
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
TEST(OSM_FactHandler, writeTagListWikidata) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, tagKey);
  const std::string object2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, "Q42");

  osm2rdf::osm::TagList tagList;
  tagList[tagKey] = tagValue;

  dh.writeTagList(subject, tagList);
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, tagKey);
  const std::string object2 = writer.generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, "Q42");

  osm2rdf::osm::TagList tagList;
  tagList[tagKey] = tagValue;

  dh.writeTagList(subject, tagList);
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, tagKey);
  const std::string object2 = "<https://de.wikipedia.org/wiki/" + value + ">";

  osm2rdf::osm::TagList tagList;
  tagList[tagKey] = tagValue;

  dh.writeTagList(subject, tagList);
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  const std::string predicate2 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, tagKey);
  const std::string object2 =
      "<https://www.wikipedia.org/wiki/" + tagValue + ">";

  osm2rdf::osm::TagList tagList;
  tagList[tagKey] = tagValue;

  dh.writeTagList(subject, tagList);
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
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
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
  const std::string predicate3 =
      writer.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, tag1Key);

  osm2rdf::osm::TagList tagList;
  tagList[tag1Key] = tag1Value;
  tagList[tag2Key] = tag2Value;

  dh.writeTagList("subject", tagList);
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
}  // namespace osm2rdf::osm
