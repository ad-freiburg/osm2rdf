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
  uint16_t simplifyGeometries = 0;

  // Select amount to dump
  bool addAreaEnvelope = false;
  bool addInverseRelationDirection = false;
  bool addSortMetadata = true;
  bool addWayEnvelope = false;
  bool addWayMetadata = false;
  bool addWayNodeOrder = false;
  bool adminRelationsOnly = false;
  bool skipWikiLinks = false;

  // Addition filters / data
  bool addAreaEnvelopeRatio = false;
  double minimalAreaEnvelopeRatio = -1.0;

  // Default settings for data
  std::string osm2ttlPrefix = "osmadd";

  // Dot
  bool writeDAGDotFiles = false;

  // Statistics
  bool writeGeometricRelationStatistics = false;
  std::filesystem::path statisticsPath;

  // Output modifiers
  uint16_t simplifyWKT = 250;
  uint16_t wktDeviation = 5;
  uint16_t wktPrecision = 7;

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

  // Parse provided commandline arguments into config object.
  void fromArgs(int argc, char** argv);

  // Generate the information string containing the current settings.
  [[nodiscard]] std::string getInfo(std::string_view prefix) const;

  // Generate a path inside the cache directory.
  [[nodiscard]] std::filesystem::path getTempPath(const std::string& p,
                                                  const std::string& s) const;
};
}  // namespace osm2ttl::config

#endif  // OSM2TTL_CONFIG_CONFIG_H_
