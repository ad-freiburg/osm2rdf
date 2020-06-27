// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONFIG_H_
#define OSM2TTL_CONFIG_CONFIG_H_

#include <string>
#include <unordered_map>

#include "osm2ttl/ttl/OutputFormat.h"

namespace osm2ttl {
namespace config {

struct Config {
  // Select what to do
  bool skipAreaPrep = false;

  // Select types to dump
  bool noNodeDump = false;
  bool noRelationDump = false;
  bool noWayDump = false;
  bool noAreaDump = false;

  // Select amount to dump
  bool addAreaSources = false;
  bool addBBox = false;
  bool addMemberNodes = false;
  bool addUnnamed = false;
  bool expandedData = false;
  bool metaData = false;
  bool skipWikiLinks = false;
  size_t simplifyWKT = 0;

  // tag.key() -> IRI
  std::unordered_map<std::string, osm2ttl::ttl::IRI> tagKeyType;

  // Output, empty for stdout
  std::string output;
  // Output format
  osm2ttl::ttl::OutputFormat outputFormat = osm2ttl::ttl::OutputFormat::QLEVER;

  // osmium location cache
  std::string cache;

  // Input file
  std::string input;

  void load(const std::string& filename);
  void save(const std::string& filename);
  void fromArgs(int argc, char** argv);

  static Config& getInstance() {
    static Config instance;
    return instance;
  }

 private:
  Config() {}
};
}  // namespace config
}  // namespace osm2ttl

#endif  // OSM2TTL_CONFIG_CONFIG_H_

