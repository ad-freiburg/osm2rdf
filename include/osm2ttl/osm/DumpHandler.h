// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_DUMPHANDLER_H_
#define OSM2TTL_OSM_DUMPHANDLER_H_

#include <ostream>

#include "gtest/gtest_prod.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/handler.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

namespace osm2ttl::osm {

template <typename W>
class DumpHandler : public osmium::handler::Handler {
 public:
  DumpHandler(const osm2ttl::config::Config& config,
              osm2ttl::ttl::Writer<W>* writer);
  void area(const osm2ttl::osm::Area& area);
  void node(const osm2ttl::osm::Node& node);
  void relation(const osm2ttl::osm::Relation& relation);
  void way(const osm2ttl::osm::Way& way);

 protected:
  template <typename G>
  void writeBoostGeometry(const std::string& s, const std::string& p,
                          const G& g);
  FRIEND_TEST(OSM_DumpHandler, writeBoostGeometry);
  void writeBox(const std::string& s, const std::string& p,
                const osm2ttl::geometry::Box& box);
  FRIEND_TEST(OSM_DumpHandler, writeBoxPrecision1);
  FRIEND_TEST(OSM_DumpHandler, writeBoxPrecision2);
  void writeTag(const std::string& s, const osm2ttl::osm::Tag& tag);
  FRIEND_TEST(OSM_DumpHandler, writeTag_AdminLevel);
  FRIEND_TEST(OSM_DumpHandler, writeTag_KeyIRI);
  FRIEND_TEST(OSM_DumpHandler, writeTag_KeyNotIRI);
  void writeTagList(const std::string& s, const osm2ttl::osm::TagList& tags);
  FRIEND_TEST(OSM_DumpHandler, writeTagList);

  const osm2ttl::config::Config _config;
  osm2ttl::ttl::Writer<W>* _writer;
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_DUMPHANDLER_H_
