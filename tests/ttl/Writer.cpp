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

#include "osm2rdf/ttl/Writer.h"

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2rdf/config/Config.h"

namespace osm2rdf::ttl {

// ____________________________________________________________________________
TEST(TTL_WriterNT, resolvePrefix) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__GEOSPARQL);
    ASSERT_STREQ("http://www.opengis.net/ont/geosparql#", res.c_str());
  }
  {
    const std::string res =
        w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE);
    ASSERT_STREQ("https://www.openstreetmap.org/node/", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, resolvePrefix) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__GEOSPARQL);
    ASSERT_STREQ("http://www.opengis.net/ont/geosparql#", res.c_str());
  }
  {
    const std::string res =
        w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE);
    ASSERT_STREQ("https://www.openstreetmap.org/node/", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, resolvePrefix) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__GEOSPARQL);
    ASSERT_STREQ("http://www.opengis.net/ont/geosparql#", res.c_str());
  }
  {
    const std::string res =
        w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE);
    ASSERT_STREQ("https://www.openstreetmap.org/node/", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, addPrefix) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.resolvePrefix("test");
    ASSERT_STREQ("test", res.c_str());
  }
  {
    w.addPrefix("test", "prefix");
    const std::string res = w.resolvePrefix("test");
    ASSERT_STREQ("prefix", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, addPrefix) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.resolvePrefix("test");
    ASSERT_STREQ("test", res.c_str());
  }
  {
    w.addPrefix("test", "prefix");
    const std::string res = w.resolvePrefix("test");
    ASSERT_STREQ("prefix", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, addPrefix) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.resolvePrefix("test");
    ASSERT_STREQ("test", res.c_str());
  }
  {
    w.addPrefix("test", "prefix");
    const std::string res = w.resolvePrefix("test");
    ASSERT_STREQ("prefix", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, writeHeader) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, &output};

  w.writeHeader();

  output.close();

  ASSERT_EQ("", buffer.str());

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, writeHeader) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> w{config, &output};

  w.writeHeader();

  output.flush();
  output.close();

  ASSERT_THAT(buffer.str(),
              ::testing::HasSubstr(
                  "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, writeHeader) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, &output};

  w.writeHeader();

  output.flush();
  output.close();

  ASSERT_THAT(buffer.str(),
              ::testing::HasSubstr(
                  "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateBlankNode) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_0", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_1", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateBlankNode) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> w{config, nullptr};
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_0", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_1", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateBlankNode) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_0", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_1", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0_2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateSkolem) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res = w.generateSkolem("1");
    ASSERT_STREQ("genid:1", res.c_str());
  }
  {
    const std::string res = w.generateSkolem("2");
    ASSERT_STREQ("genid:2", res.c_str());
  }
  {
    const std::string res = w.generateSkolem("3");
    ASSERT_STREQ("genid:3", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateSkolemForRelationMember) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res = w.generateSkolemForRelationMember(1, 2, "n");
    ASSERT_STREQ("genid:r1n2", res.c_str());
  }
  {
    const std::string res = w.generateSkolemForRelationMember(1, 2, "w");
    ASSERT_STREQ("genid:r1w2", res.c_str());
  }
  {
    const std::string res = w.generateSkolemForRelationMember(1, 2, "r");
    ASSERT_STREQ("genid:r1r2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateSkolemForWayMember) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res = w.generateSkolemForWayMember(1, 1);
    ASSERT_STREQ("genid:w1n1", res.c_str());
  }
  {
    const std::string res = w.generateSkolemForWayMember(2, 3);
    ASSERT_STREQ("genid:w2n3", res.c_str());
  }
  {
    const std::string res = w.generateSkolemForWayMember(3, 5);
    ASSERT_STREQ("genid:w3n5", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateIRI_ID) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, 23);
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE) +
         "23>")
            .c_str(),
        res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", 42);
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "42>").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", 1337);
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "1337>").c_str(),
                 res.c_str());
  }
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__RDF, 42);
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__RDF) + "42>")
            .c_str(),
        res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateIRI_ID) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, 23);
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__OSM_NODE + ":23").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", 42);
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "42>").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", 1337);
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "1337>").c_str(),
                 res.c_str());
  }
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__RDF, 42);
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__RDF + ":42").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateIRI_ID) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, 23);
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__OSM_NODE + ":23").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", 42);
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "42>").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", 1337);
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "1337>").c_str(),
                 res.c_str());
  }
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__RDF, 42);
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__RDF + ":42").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateIRI_String) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, "a");
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE) +
         "a>")
            .c_str(),
        res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", "x");
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "x>").c_str(), res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", "e");
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "e>").c_str(), res.c_str());
  }
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__RDF, "l");
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2rdf::ttl::constants::NAMESPACE__RDF) + "l>")
            .c_str(),
        res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateIRI_String) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, "a");
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__OSM_NODE + ":a").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", "x");
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "x>").c_str(), res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", "e");
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "e>").c_str(), res.c_str());
  }
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__RDF, "l");
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__RDF + ":l").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateIRI_String) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, "a");
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__OSM_NODE + ":a").c_str(),
                 res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", "x");
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "x>").c_str(), res.c_str());
  }
  {
    const std::string res = w.generateIRI("prefix", "e");
    ASSERT_STREQ(("<" + w.resolvePrefix("prefix") + "e>").c_str(), res.c_str());
  }
  {
    const std::string res =
        w.generateIRI(osm2rdf::ttl::constants::NAMESPACE__RDF, "l");
    ASSERT_STREQ((osm2rdf::ttl::constants::NAMESPACE__RDF + ":l").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateLangTag) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("2"), std::domain_error); }
  { ASSERT_THROW(w.generateLangTag("-Three"), std::domain_error); }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("Five5"), std::domain_error); }
  {
    const std::string res = w.generateLangTag("SIX-6");
    ASSERT_STREQ("@SIX-6", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Seven-7-seven");
    ASSERT_STREQ("@Seven-7-seven", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("Eight§"), std::domain_error); }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateLangTag) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("2"), std::domain_error); }
  { ASSERT_THROW(w.generateLangTag("-Three"), std::domain_error); }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("Five5"), std::domain_error); }
  {
    const std::string res = w.generateLangTag("SIX-6");
    ASSERT_STREQ("@SIX-6", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Seven-7-seven");
    ASSERT_STREQ("@Seven-7-seven", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("Eight§"), std::domain_error); }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateLangTag) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("2"), std::domain_error); }
  { ASSERT_THROW(w.generateLangTag("-Three"), std::domain_error); }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("Five5"), std::domain_error); }
  {
    const std::string res = w.generateLangTag("SIX-6");
    ASSERT_STREQ("@SIX-6", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Seven-7-seven");
    ASSERT_STREQ("@Seven-7-seven", res.c_str());
  }
  { ASSERT_THROW(w.generateLangTag("Eight§"), std::domain_error); }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateLiteral) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLiteral("one", "");
    ASSERT_STREQ("\"one\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("2", "");
    ASSERT_STREQ("\"2\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("-", "Three");
    ASSERT_STREQ("\"-\"Three", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("Four-four", "^^Four");
    ASSERT_STREQ("\"Four-four\"^^Four", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("\ufafaFive", "");
    ASSERT_STREQ("\"\ufafaFive\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("SIX-6", "\ufafa");
    ASSERT_STREQ("\"SIX-6\"\ufafa", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateLiteral) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLiteral("one", "");
    ASSERT_STREQ("\"one\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("2", "");
    ASSERT_STREQ("\"2\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("-", "Three");
    ASSERT_STREQ("\"-\"Three", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("Four-four", "^^Four");
    ASSERT_STREQ("\"Four-four\"^^Four", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("\ufafaFive", "");
    ASSERT_STREQ("\"\ufafaFive\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("SIX-6", "\ufafa");
    ASSERT_STREQ("\"SIX-6\"\ufafa", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateLiteral) {
  osm2rdf::config::Config config;
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLiteral("one", "");
    ASSERT_STREQ("\"one\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("2", "");
    ASSERT_STREQ("\"2\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("-", "Three");
    ASSERT_STREQ("\"-\"Three", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("Four-four", "^^Four");
    ASSERT_STREQ("\"Four-four\"^^Four", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("\ufafaFive", "");
    ASSERT_STREQ("\"\ufafaFive\"", res.c_str());
  }
  {
    const std::string res = w.generateLiteral("SIX-6", "\ufafa");
    ASSERT_STREQ("\"SIX-6\"\ufafa", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, writeStatisticJson) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> w{config, &output};

  // Setup temp dir and stats file
  std::filesystem::path tmpDir =
      config.getTempPath("TEST_TTL_WriterNT", "writeStatisticJson");
  ASSERT_FALSE(std::filesystem::exists(tmpDir));
  std::filesystem::create_directories(tmpDir);
  ASSERT_TRUE(std::filesystem::exists(tmpDir));
  ASSERT_TRUE(std::filesystem::is_directory(tmpDir));
  std::filesystem::path statsFile{tmpDir};
  statsFile /= "file";

  // This is empty for NT
  w.writeHeader();

  // 3 blank nodes
  w.generateBlankNode();
  w.generateBlankNode();
  w.generateBlankNode();

  // Add 4 lines
  w.writeTriple("s0", "p", "o");
  w.writeTriple("s1", "p", "o");
  w.writeTriple("s2", "p", "o");
  w.writeTriple("s3", "p", "o");
  w.writeTriple("s4", "p", "o");

  w.writeStatisticJson(statsFile);

  output.close();

  // Read file
  std::ifstream statsIFStream(statsFile);
  std::stringstream statsBuffer;
  statsBuffer << statsIFStream.rdbuf();

  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"blankNodes\": 3"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"header\": 0"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"lines\": 5"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"triples\": 5"));

  // Cleanup
  statsIFStream.close();
  std::cout.rdbuf(sbuf);
  std::filesystem::remove_all(tmpDir);
  ASSERT_FALSE(std::filesystem::exists(tmpDir));
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, writeStatisticJson) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> w{config, &output};

  // Setup temp dir and stats file
  std::filesystem::path tmpDir =
      config.getTempPath("TEST_TTL_WriterTTL", "writeStatisticJson");
  std::filesystem::remove_all(tmpDir);
  ASSERT_FALSE(std::filesystem::exists(tmpDir));
  std::filesystem::create_directories(tmpDir);
  ASSERT_TRUE(std::filesystem::exists(tmpDir));
  ASSERT_TRUE(std::filesystem::is_directory(tmpDir));
  std::filesystem::path statsFile{tmpDir};
  statsFile /= "file";

  // This writes a line for each entry in w._prefixes
  w.writeHeader();

  // 3 blank nodes
  w.generateBlankNode();
  w.generateBlankNode();
  w.generateBlankNode();

  // Add 4 lines
  w.writeTriple("s0", "p", "o");
  w.writeTriple("s1", "p", "o");
  w.writeTriple("s2", "p", "o");
  w.writeTriple("s3", "p", "o");
  w.writeTriple("s4", "p", "o");

  w.writeStatisticJson(statsFile);

  output.close();

  // Read file
  std::ifstream statsIFStream(statsFile);
  std::stringstream statsBuffer;
  statsBuffer << statsIFStream.rdbuf();

  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"blankNodes\": 3"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"header\": 22"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"lines\": 27"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"triples\": 5"));

  // Cleanup
  statsIFStream.close();
  std::cout.rdbuf(sbuf);
  std::filesystem::remove_all(tmpDir);
  ASSERT_FALSE(std::filesystem::exists(tmpDir));
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, writeStatisticJson) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> w{config, &output};

  // Setup temp dir and stats file
  std::filesystem::path tmpDir =
      config.getTempPath("TEST_TTL_WriterQLEVER", "writeStatisticJson");
  std::filesystem::remove_all(tmpDir);
  ASSERT_FALSE(std::filesystem::exists(tmpDir));
  std::filesystem::create_directories(tmpDir);
  ASSERT_TRUE(std::filesystem::exists(tmpDir));
  ASSERT_TRUE(std::filesystem::is_directory(tmpDir));
  std::filesystem::path statsFile{tmpDir};
  statsFile /= "file";

  // This writes a line for each entry in w._prefixes
  w.writeHeader();

  // 3 blank nodes
  w.generateBlankNode();
  w.generateBlankNode();
  w.generateBlankNode();

  // Add 4 lines
  w.writeTriple("s0", "p", "o");
  w.writeTriple("s1", "p", "o");
  w.writeTriple("s2", "p", "o");
  w.writeTriple("s3", "p", "o");
  w.writeTriple("s4", "p", "o");

  w.writeStatisticJson(statsFile);

  output.close();

  // Read file
  std::ifstream statsIFStream(statsFile);
  std::stringstream statsBuffer;
  statsBuffer << statsIFStream.rdbuf();

  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"blankNodes\": 3"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"header\": 22"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"lines\": 27"));
  ASSERT_THAT(statsBuffer.str(), ::testing::HasSubstr("\"triples\": 5"));

  // Cleanup
  statsIFStream.close();
  std::cout.rdbuf(sbuf);
  std::filesystem::remove_all(tmpDir);
  ASSERT_FALSE(std::filesystem::exists(tmpDir));
}

}  // namespace osm2rdf::ttl
