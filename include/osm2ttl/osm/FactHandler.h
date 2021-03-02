// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_FACTHANDLER_H_
#define OSM2TTL_OSM_FACTHANDLER_H_

#include <ostream>

#include "gtest/gtest_prod.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/ttl/Writer.h"

namespace osm2ttl::osm {

const double ONE_PERCENT = 0.01;

template <typename W>
class FactHandler {
 public:
  FactHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer<W>* writer);
  void area(const osm2ttl::osm::Area& area);
  void node(const osm2ttl::osm::Node& node);
  void relation(const osm2ttl::osm::Relation& relation);
  void way(const osm2ttl::osm::Way& way);

 protected:
  template <typename G>
  void writeBoostGeometry(const std::string& s, const std::string& p,
                          const G& g);
  FRIEND_TEST(OSM_FactHandler, writeBoostGeometryWay);
  FRIEND_TEST(OSM_FactHandler, writeBoostGeometryWaySimplify1);
  FRIEND_TEST(OSM_FactHandler, writeBoostGeometryWaySimplify2);
  FRIEND_TEST(OSM_FactHandler, writeBoostGeometryWaySimplify3);
  void writeBox(const std::string& s, const std::string& p,
                const osm2ttl::geometry::Box& box);
  FRIEND_TEST(OSM_FactHandler, writeBoxPrecision1);
  FRIEND_TEST(OSM_FactHandler, writeBoxPrecision2);
  void writeTag(const std::string& s, const osm2ttl::osm::Tag& tag);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel);
  FRIEND_TEST(OSM_FactHandler, writeTag_KeyIRI);
  FRIEND_TEST(OSM_FactHandler, writeTag_KeyNotIRI);
  void writeTagList(const std::string& s, const osm2ttl::osm::TagList& tags);
  FRIEND_TEST(OSM_FactHandler, writeTagList);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikidata);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikidataMultiple);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikipediaWithLang);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikipediaWithoutLang);
  FRIEND_TEST(OSM_FactHandler, writeTagListSkipWikiLinks);

  const osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_FACTHANDLER_H_
