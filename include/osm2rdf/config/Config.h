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

#ifndef OSM2RDF_CONFIG_CONFIG_H_
#define OSM2RDF_CONFIG_CONFIG_H_

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

#include "osm2rdf/config/Constants.h"
#include "osm2rdf/ttl/Format.h"
#include "osm2rdf/util/OutputMergeMode.h"

namespace osm2rdf::config {

enum GeoTriplesMode {
  none = 0,
  full = 1
};

enum SourceDataset {
  OSM = 0,
  OHM = 1
};

struct Config {
  // Select what to do
  std::string storeLocationsOnDisk;

  bool noFacts = false;
  bool noAreaFacts = false;
  bool noNodeFacts = false;
  bool noRelationFacts = false;
  bool noWayFacts = false;

  bool noGeometricRelations = false;
  bool noAreaGeometricRelations = false;
  bool noNodeGeometricRelations = false;
  bool noRelationGeometricRelations = false;
  bool noWayGeometricRelations = false;
  double simplifyGeometries = 0;

  SourceDataset sourceDataset = OSM;

  // Select amount to dump
  bool addAreaWayLinestrings = false;
  bool addCentroids = true;
  bool addWayMetadata = false;
  bool addWayNodeGeometry = false;
  bool addWayNodeOrder = false;
  bool addWayNodeSpatialMetadata = false;
  bool skipWikiLinks = false;

  // Default settings for data
  std::unordered_set<std::string> semicolonTagKeys;

  // Auxilary geo files
  std::vector<std::string> auxGeoFiles;

  // Statistics
  bool writeRDFStatistics = false;
  std::filesystem::path rdfStatisticsPath;

  // Output modifiers
  uint16_t simplifyWKT = 0;
  double wktDeviation = 5;
  uint16_t wktPrecision = 7;

  GeoTriplesMode ogcGeoTriplesMode = full;

  // Output, empty for stdout
  std::filesystem::path output;
  std::string outputFormat = "qlever";
  osm2rdf::util::OutputMergeMode mergeOutput =
      osm2rdf::util::OutputMergeMode::CONCATENATE;
  bool outputCompress = true;
  bool outputKeepFiles = false;

  // osmium location cache
  std::filesystem::path cache{std::filesystem::temp_directory_path()};

  // Input file
  std::filesystem::path input;

  // Parse provided commandline arguments into config object.
  void fromArgs(int argc, char** argv);

  // Generate the information string containing the current settings.
  [[nodiscard]] std::string getInfo(std::string_view prefix) const;

  // Generate a path inside the cache directory.
  [[nodiscard]] std::filesystem::path getTempPath(
      const std::string& path, const std::string& suffix) const;
};
}  // namespace osm2rdf::config

#endif  // OSM2RDF_CONFIG_CONFIG_H_
