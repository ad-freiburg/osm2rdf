// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/ttl/Writer.h"

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2ttl/config/Config.h"

namespace osm2ttl::ttl {

// ____________________________________________________________________________
TEST(TTL_WriterNT, resolvePrefix) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM);
    ASSERT_STREQ("https://www.openstreetmap.org/", res.c_str());
  }
  {
    const std::string res =
        w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE);
    ASSERT_STREQ("https://www.openstreetmap.org/node/", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, resolvePrefix) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM);
    ASSERT_STREQ("https://www.openstreetmap.org/", res.c_str());
  }
  {
    const std::string res =
        w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE);
    ASSERT_STREQ("https://www.openstreetmap.org/node/", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, resolvePrefix) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM);
    ASSERT_STREQ("https://www.openstreetmap.org/", res.c_str());
  }
  {
    const std::string res =
        w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE);
    ASSERT_STREQ("https://www.openstreetmap.org/node/", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, addPrefix) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
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
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
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
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, &output};

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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config, &output};

  w.writeHeader();

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

  osm2ttl::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = util::OutputMergeMode::NONE;
  osm2ttl::util::Output output{config, config.output};
  output.open();
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config, &output};

  w.writeHeader();

  output.close();

  ASSERT_THAT(buffer.str(),
              ::testing::HasSubstr(
                  "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .\n"));

  // Cleanup
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateBlankNode) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:1", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateBlankNode) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config, nullptr};
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:1", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateBlankNode) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:0", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:1", res.c_str());
  }
  {
    const std::string res = w.generateBlankNode();
    ASSERT_STREQ("_:2", res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateIRI_ID) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, 23);
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE) +
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
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, 42);
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__RDF) + "42>")
            .c_str(),
        res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateIRI_ID) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, 23);
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__OSM_NODE + ":23").c_str(),
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
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, 42);
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__RDF + ":42").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateIRI_ID) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, 23);
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__OSM_NODE + ":23").c_str(),
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
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, 42);
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__RDF + ":42").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateIRI_String) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, "a");
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE) +
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
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, "l");
    ASSERT_STREQ(
        ("<" + w.resolvePrefix(osm2ttl::ttl::constants::NAMESPACE__RDF) + "l>")
            .c_str(),
        res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateIRI_String) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::TTL> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, "a");
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__OSM_NODE + ":a").c_str(),
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
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, "l");
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__RDF + ":l").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateIRI_String) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::QLEVER> w{config, nullptr};
  {
    const std::string res =
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, "a");
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__OSM_NODE + ":a").c_str(),
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
        w.generateIRI(osm2ttl::ttl::constants::NAMESPACE__RDF, "l");
    ASSERT_STREQ((osm2ttl::ttl::constants::NAMESPACE__RDF + ":l").c_str(),
                 res.c_str());
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateLangTag) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("2"), std::domain_error);
  }
  {
    ASSERT_THROW(w.generateLangTag("-Three"), std::domain_error);
  }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("Five5"), std::domain_error);
  }
  {
    const std::string res = w.generateLangTag("SIX-6");
    ASSERT_STREQ("@SIX-6", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Seven-7-seven");
    ASSERT_STREQ("@Seven-7-seven", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("Eight§"), std::domain_error);
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterTTL, generateLangTag) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("2"), std::domain_error);
  }
  {
    ASSERT_THROW(w.generateLangTag("-Three"), std::domain_error);
  }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("Five5"), std::domain_error);
  }
  {
    const std::string res = w.generateLangTag("SIX-6");
    ASSERT_STREQ("@SIX-6", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Seven-7-seven");
    ASSERT_STREQ("@Seven-7-seven", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("Eight§"), std::domain_error);
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterQLEVER, generateLangTag) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("2"), std::domain_error);
  }
  {
    ASSERT_THROW(w.generateLangTag("-Three"), std::domain_error);
  }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("Five5"), std::domain_error);
  }
  {
    const std::string res = w.generateLangTag("SIX-6");
    ASSERT_STREQ("@SIX-6", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Seven-7-seven");
    ASSERT_STREQ("@Seven-7-seven", res.c_str());
  }
  {
    ASSERT_THROW(w.generateLangTag("Eight§"), std::domain_error);
  }
}

// ____________________________________________________________________________
TEST(TTL_WriterNT, generateLiteral) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
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
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
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
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
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

}  // namespace osm2ttl::ttl
