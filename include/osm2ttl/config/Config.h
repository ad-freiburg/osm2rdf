// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONFIG_H_
#define OSM2TTL_CONFIG_CONFIG_H_

#include <filesystem>
#include <string>
#include <unordered_map>

#include "osm2ttl/ttl/Format.h"

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

  // Output modifiers
  uint16_t wktSimplify = 100;
  uint8_t wktDeviation = 5;
  uint8_t wktPrecision = 12;
  bool gzip = false;

  // Threads
  size_t numThreadsConvertGeometry = 4;
  size_t queueFactorConvertGeometry = 2;
  size_t numThreadsConvertString = 4;
  size_t queueFactorConvertString = 2;
  size_t numThreadsRead = 2;
  size_t queueFactorRead = 2;

  // Output, empty for stdout
  std::filesystem::path output;
  std::string outputFormat;

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

