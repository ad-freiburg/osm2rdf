// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/DumpHandler.h"

#include "gtest/gtest.h"
#include "osm2ttl/osm/Node.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

TEST(OSM_DumpHandler, constructor) {
  osm2ttl::config::Config config;
  config.output = config.getTempPath("TEST_OSM_DumpHandler", "constructor");
  std::filesystem::create_directories(config.output);
  osm2ttl::util::Output output{config, config.output};
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  // Assure that no file is written during construction.
  ASSERT_EQ(0, std::distance(std::filesystem::directory_iterator(config.output),
                             std::filesystem::directory_iterator()));

  // Cleanup
  output.close();
  std::filesystem::remove_all(config.output);
}

TEST(OSM_DumpHandler, areaFromWay) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};

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

TEST(OSM_DumpHandler, areaFromRelation) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};

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

TEST(OSM_DumpHandler, areaAddEnvelope) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addAreaEnvelope = true;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};

  dh.area(a);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:21 geo:hasGeometry \"MULTIPOLYGON(((48.0 7.5,48.0 7.6,48.1 "
      "7.6,48.1 7.5,48.0 7.5)))\"^^geo:wktLiteral .\n"
      "osmway:21 osm:envelope \"POLYGON((48.0 7.5,48.0 7.6,48.1 7.6,48.1 "
      "7.5,48.0 7.5))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, node) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};

  dh.node(n);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmnode:42 rdf:type osm:node .\n"
      "osmnode:42 geo:hasGeometry \"POINT(7.5 48.0)\"^^geo:wktLiteral .\n"
      "osmnode:42 osmt:city \"Freiburg\" .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, relation) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation r{osmiumBuffer.get<osmium::Relation>(0)};

  dh.relation(r);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmrel:42 rdf:type osm:relation .\n"
      "osmrel:42 osmt:city \"Freiburg\" .\n"
      "osmrel:42 osmrel:member _:0 .\n"
      "_:0 osm:id osmnode:1 .\n"
      "_:0 osm:role \"osmnode\" .\n"
      "osmrel:42 osmrel:member _:1 .\n"
      "_:1 osm:id osmrel:1 .\n"
      "_:1 osm:role \"osmrel\" .\n"
      "osmrel:42 osmrel:member _:2 .\n"
      "_:2 osm:id osm:1 .\n"
      "_:2 osm:role \"osm\" .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, way) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmt:city \"Freiburg\" .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, wayAddWayEnvelope) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addWayEnvelope = true;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmt:city \"Freiburg\" .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osm:envelope \"POLYGON((48.0 7.5,48.0 7.6,48.1 7.6,48.1 "
      "7.5,48.0 7.5))\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, wayAddWayNodeOrder) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addWayNodeOrder = true;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmt:city \"Freiburg\" .\n"
      "osmway:42 osmway:node _:0 .\n"
      "_:0 osmway:node osmnode:1 .\n"
      "_:0 osmm:pos \"1\"^^xsd:integer .\n"
      "osmway:42 osmway:node _:1 .\n"
      "_:1 osmway:node osmnode:2 .\n"
      "_:1 osmm:pos \"2\"^^xsd:integer .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, wayAddWayMetaData) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;
  config.addWayMetaData = true;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

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

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Way w{osmiumBuffer.get<osmium::Way>(0)};

  dh.way(w);
  output.flush();
  output.close();

  ASSERT_EQ(
      "osmway:42 rdf:type osm:way .\n"
      "osmway:42 osmt:city \"Freiburg\" .\n"
      "osmway:42 geo:hasGeometry \"LINESTRING(48.0 7.5,48.1 "
      "7.6)\"^^geo:wktLiteral .\n"
      "osmway:42 osmway:is_closed \"no\" .\n"
      "osmway:42 osmway:nodeCount \"2\"^^xsd:integer .\n"
      "osmway:42 osmway:uniqueNodeCount \"2\"^^xsd:integer .\n",
      buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, writeBoxPrecision1) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 1;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  osm2ttl::geometry::Box box;
  box.min_corner() = osm2ttl::geometry::Location{50, 50};
  box.max_corner() = osm2ttl::geometry::Location{200, 200};

  dh.writeBox(subject, predicate, box);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"POLYGON((50.0 50.0,50.0 200.0,200.0 200.0,200.0 50.0,50.0 "
                "50.0))\"" +
                "^^" + osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL +
                " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, writeBoxPrecision2) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = osm2ttl::util::OutputMergeMode::NONE;
  config.wktPrecision = 2;

  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  const std::string subject = "subject";
  const std::string predicate = "predicate";
  osm2ttl::geometry::Box box;
  box.min_corner() = osm2ttl::geometry::Location{50, 50};
  box.max_corner() = osm2ttl::geometry::Location{200, 200};

  dh.writeBox(subject, predicate, box);
  output.flush();
  output.close();

  ASSERT_EQ(subject + " " + predicate + " " +
                "\"POLYGON((50.00 50.00,50.00 200.00,200.00 200.00,200.00 "
                "50.00,50.00 50.00))\"" +
                "^^" + osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL +
                " .\n",
            buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, writeTag_AdminLevel) {
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
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  const std::string tagKey = "admin_level";
  const std::string tagValue = "42";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(
      tagValue, "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER);
  dh.writeTag(subject, osm2ttl::osm::Tag{tagKey, tagValue});
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, writeTag_KeyIRI) {
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
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  const std::string tagKey = "iri";
  const std::string tagValue = "value";

  const std::string subject = "subject";
  const std::string predicate =
      writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, tagKey);
  const std::string object = writer.generateLiteral(tagValue, "");
  dh.writeTag(subject, osm2ttl::osm::Tag{tagKey, tagValue});
  const std::string expected =
      subject + " " + predicate + " " + object + " .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, writeTag_KeyNotIRI) {
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
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  const std::string tagKey = "not:Aß%I.R.I\u2000";
  const std::string tagValue = "value";

  const std::string subject = "subject";
  dh.writeTag(subject, osm2ttl::osm::Tag{tagKey, tagValue});
  const std::string expected = subject +
                               " osm:tag _:0 .\n"
                               "_:0 osmt:key \"" +
                               tagKey +
                               "\" .\n"
                               "_:0 osmt:value \"" +
                               tagValue + "\" .\n";
  output.flush();
  output.close();

  ASSERT_EQ(expected, buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

TEST(OSM_DumpHandler, writeTagList) {
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
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> writer{config, &output};
  osm2ttl::osm::DumpHandler dh{config, &writer};

  const std::string tag1Key = "admin_level";
  const std::string tag1Value = "42";
  const std::string tag2Key = "iri";
  const std::string tag2Value = "value";

  const std::string subject = "subject";
  const std::string predicate1 =
      writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, tag1Key);
  const std::string object1 = writer.generateLiteral(
      tag1Value, "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER);
  const std::string predicate2 =
      writer.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, tag2Key);
  const std::string object2 = writer.generateLiteral(tag2Value, "");

  osm2ttl::osm::TagList tagList;
  tagList[tag1Key] = tag1Value;
  tagList[tag2Key] = tag2Value;

  dh.writeTagList(subject, tagList);
  output.flush();
  output.close();

  const std::string printedData = buffer.str();
  bool order1Found =
      printedData == (subject + " " + predicate1 + " " + object1 + " .\n" +
                      subject + " " + predicate2 + " " + object2 + " .\n");
  bool order2Found =
      printedData == (subject + " " + predicate2 + " " + object2 + " .\n" +
                      subject + " " + predicate1 + " " + object1 + " .\n");

  ASSERT_TRUE(order1Found | order2Found);

  // Cleanup
  std::cout.rdbuf(sbuf);
}
}  // namespace osm2ttl::osm