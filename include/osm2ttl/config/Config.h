// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONFIG_H_
#define OSM2TTL_CONFIG_CONFIG_H_

#include <filesystem>
#include <string>
#include <unordered_map>

#include "osm2ttl/config/Constants.h"
#include "osm2ttl/ttl/Format.h"
#include "osm2ttl/util/OutputMergeMode.h"

namespace osm2ttl::config {

struct Config {
  // Select what to do
  bool storeLocationsOnDisk = false;

  bool noFacts = false;
  bool noAreaFacts = false;
  bool noNodeFacts = false;
  bool noRelationFacts = false;
  bool noWayFacts = false;

  bool noGeometricRelations = false;
  bool noAreaGeometricRelations = false;
  bool noNodeGeometricRelations = false;
  bool noWayGeometricRelations = false;

  // Select amount to dump
  bool addAreaEnvelope = false;
  bool addInverseRelationDirection = false;
  bool addSortMetadata = true;
  bool addWayEnvelope = false;
  bool addWayMetadata = false;
  bool addWayNodeOrder = false;
  bool adminRelationsOnly = false;
  bool skipWikiLinks = false;

  // Default settings for data
  std::string osm2ttlPrefix = "osmadd";

  // Dot
  bool writeDAGDotFiles = false;

  // Statistics
  bool writeGeometricRelationStatistics = false;
  std::filesystem::path statisticsPath;

  // Output modifiers
  uint16_t wktSimplify = 250;
  uint8_t wktDeviation = 5;
  uint8_t wktPrecision = 8;

  // Output, empty for stdout
  std::filesystem::path output;
  std::string outputFormat;
  osm2ttl::util::OutputMergeMode mergeOutput =
      osm2ttl::util::OutputMergeMode::CONCATENATE;
  bool outputCompress = true;

  // osmium location cache
  std::filesystem::path cache{std::filesystem::temp_directory_path()};

  // Input file
  std::filesystem::path input;

  void fromArgs(int argc, char** argv);
  [[nodiscard]] std::string getInfo(std::string_view prefix) const;
  [[nodiscard]] std::filesystem::path getTempPath(const std::string& p,
                                                  const std::string& s) const;
};
}  // namespace osm2ttl::config

#endif  // OSM2TTL_CONFIG_CONFIG_H_
