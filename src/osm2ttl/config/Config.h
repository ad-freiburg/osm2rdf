// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONFIG_H_
#define OSM2TTL_CONFIG_CONFIG_H_

#include <filesystem>

#include <string>
#include <unordered_map>

#include "osm2ttl/ttl/OutputFormat.h"

namespace osm2ttl {
namespace config {

struct Config {
  // Select what to do
  bool skipAreaPrep = false;
  bool useRamForLocations = false;

  // Select types to dump
  bool noNodeDump = false;
  bool noRelationDump = false;
  bool noWayDump = false;
  bool noAreaDump = false;

  // Select amount to dump
  bool addAreaSources = false;
  bool addEnvelope = false;
  bool addMemberNodes = false;
  bool expandedData = false;
  bool metaData = false;
  bool skipWikiLinks = false;
  size_t simplifyWKT = 0;

  size_t writerThreads = 6;
  size_t dumpThreads = 4;

  // tag.key() -> IRI
  std::unordered_map<std::string, osm2ttl::ttl::IRI> tagKeyType;

  // Output, empty for stdout
  std::filesystem::path output;
  // Output format
  osm2ttl::ttl::OutputFormat outputFormat = osm2ttl::ttl::OutputFormat::QLEVER;

  // osmium location cache
  std::filesystem::path cache;

  // Input file
  std::filesystem::path input;

  void load(const std::string& filename);
  void save(const std::string& filename);
  void fromArgs(int argc, char** argv);
  std::filesystem::path getTempPath(const std::string& p,
                                    const std::string& s) const;

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

