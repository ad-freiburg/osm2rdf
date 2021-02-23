// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/Writer.h"

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
    const std::string res = w.generateLangTag("2");
    ASSERT_STREQ("@", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("-Three");
    ASSERT_STREQ("@", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Five5");
    ASSERT_STREQ("@", res.c_str());
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
    const std::string res = w.generateLangTag("Eight§");
    ASSERT_STREQ("@", res.c_str());
  }
}

TEST(TTL_WriterTTL, generateLangTag) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("2");
    ASSERT_STREQ("@", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("-Three");
    ASSERT_STREQ("@", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Five5");
    ASSERT_STREQ("@", res.c_str());
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
    const std::string res = w.generateLangTag("Eight§");
    ASSERT_STREQ("@", res.c_str());
  }
}

TEST(TTL_WriterQLEVER, generateLangTag) {
  osm2ttl::config::Config config;
  osm2ttl::ttl::Writer<osm2ttl::ttl::format::NT> w{config, nullptr};
  {
    const std::string res = w.generateLangTag("one");
    ASSERT_STREQ("@one", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("2");
    ASSERT_STREQ("@", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("-Three");
    ASSERT_STREQ("@", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Four-four");
    ASSERT_STREQ("@Four-four", res.c_str());
  }
  {
    const std::string res = w.generateLangTag("Five5");
    ASSERT_STREQ("@", res.c_str());
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
    const std::string res = w.generateLangTag("Eight§");
    ASSERT_STREQ("@", res.c_str());
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
