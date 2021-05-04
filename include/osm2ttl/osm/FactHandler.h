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

#ifndef OSM2TTL_OSM_FACTHANDLER_H_
#define OSM2TTL_OSM_FACTHANDLER_H_

#include <ostream>

#include "gtest/gtest_prod.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/ttl/Writer.h"

namespace osm2ttl::osm {

template <typename W>
class FactHandler {
 public:
  FactHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer<W>* writer);
  // Add data
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
  FRIEND_TEST(OSM_FactHandler, writeTagListRefSingle);
  FRIEND_TEST(OSM_FactHandler, writeTagListRefMultiple);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikidataMultiple);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikipediaWithLang);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikipediaWithoutLang);
  FRIEND_TEST(OSM_FactHandler, writeTagListSkipWikiLinks);

  const osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_FACTHANDLER_H_
