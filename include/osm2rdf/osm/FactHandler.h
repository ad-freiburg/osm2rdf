// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#ifndef OSM2RDF_OSM_FACTHANDLER_H_
#define OSM2RDF_OSM_FACTHANDLER_H_

#include <ostream>

#include "gtest/gtest_prod.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/ttl/Writer.h"
#include "util/geo/Geo.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Box.h"
#include "osm2rdf/osm/Changeset.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/Tag.h"
#include "osm2rdf/osm/TagList.h"

namespace osm2rdf::osm {

enum DateTimeType {
  invalid = 0,
  date_yyyy = 1,
  date_yyyy_mm = 2,
  date_yyyy_mm_dd = 3,
  date_time = 4
};

template <typename W>
class FactHandler {
 public:
  FactHandler(const osm2rdf::config::Config& config,
              osm2rdf::ttl::Writer<W>* writer);
  // Add data
  void area(const osm2rdf::osm::Area& area);
  void changeset(const osm2rdf::osm::Changeset& changeset);
  void node(const osm2rdf::osm::Node& node);
  void relation(const osm2rdf::osm::Relation& relation);
  void way(const osm2rdf::osm::Way& way);

  template <typename G>
  void writeGeometry(const std::string& s, const std::string& p,
                          const G& g);

 protected:
  void writeBox(const std::string& s, const std::string& p,
                const ::util::geo::DBox& box);
  FRIEND_TEST(OSM_FactHandler, writeBoxPrecision1);
  FRIEND_TEST(OSM_FactHandler, writeBoxPrecision2);

  void writeTag(const std::string& s, const osm2rdf::osm::Tag& tag);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_nonInteger);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_nonInteger2);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_nonInteger3);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_Integer);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerPositive);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerNegative);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerWS);
  FRIEND_TEST(OSM_FactHandler, writeTag_AdminLevel_IntegerWS2);
  FRIEND_TEST(OSM_FactHandler, writeTag_KeyIRI);
  FRIEND_TEST(OSM_FactHandler, writeTag_KeyNotIRI);

  void writeTagList(const std::string& s, const osm2rdf::osm::TagList& tags);
  FRIEND_TEST(OSM_FactHandler, writeTagList);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikidata);
  FRIEND_TEST(OSM_FactHandler, writeTagListRefSingle);
  FRIEND_TEST(OSM_FactHandler, writeTagListRefDouble);
  FRIEND_TEST(OSM_FactHandler, writeTagListRefMultiple);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikidataMultiple);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikipediaWithLang);
  FRIEND_TEST(OSM_FactHandler, writeTagListWikipediaWithoutLang);
  FRIEND_TEST(OSM_FactHandler, writeTagListSkipWikiLinks);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateInvalid);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateInvalid2);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateInvalid3);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYear1);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYear2);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYear3);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYear4);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonth1);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonth2);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonth3);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonth4);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonth5);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay1);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay2);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay3);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay4);
  FRIEND_TEST(OSM_FactHandler, writeTagListStartDateYearMonthDay5);

  void writeSecondsAsISO(const std::string& s, const std::string& p,
                         const std::time_t& t);
  FRIEND_TEST(OSM_FactHandler, writeSecondsAsISO);

  bool hasSuffix(const std::string& s, const std::string& suffix) const;

  const osm2rdf::config::Config _config;
  osm2rdf::ttl::Writer<W>* _writer;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_FACTHANDLER_H_
