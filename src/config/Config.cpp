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

#include <filesystem>
#include <iostream>
#include <string>

#include "osm2rdf/config/Config.h"

#if defined(_OPENMP)
#include "omp.h"
#endif
#include "osm2rdf/config/Constants.h"
#include "osm2rdf/config/ExitCode.h"
#include "popl.hpp"

// ____________________________________________________________________________
std::string osm2rdf::config::Config::getInfo(std::string_view prefix) const {
  std::ostringstream oss;
  std::string datasetStrings[2] = {"OSM", "OHM"};
  oss << prefix << osm2rdf::config::constants::HEADER;
  oss << "\n" << prefix << osm2rdf::config::constants::SECTION_IO;
  oss << "\n"
      << prefix << osm2rdf::config::constants::INPUT_INFO << "          "
      << input;
  oss << "\n"
      << prefix << osm2rdf::config::constants::SOURCE_DATASET_INFO << ": "
      << (datasetStrings[sourceDataset]);
  oss << "\n"
      << prefix << osm2rdf::config::constants::OUTPUT_INFO << "         "
      << output;
  oss << "\n"
      << prefix << osm2rdf::config::constants::OUTPUT_FORMAT_INFO << "  "
      << outputFormat;
  oss << "\n"
      << prefix << osm2rdf::config::constants::CACHE_INFO << "          "
      << cache;
  oss << "\n" << prefix << osm2rdf::config::constants::SECTION_FACTS;
  if (noFacts) {
    oss << "\n" << prefix << osm2rdf::config::constants::NO_FACTS_INFO;
  } else {
    if (noAreaFacts) {
      oss << "\n" << prefix << osm2rdf::config::constants::NO_AREA_FACTS_INFO;
    }
    if (noNodeFacts) {
      oss << "\n" << prefix << osm2rdf::config::constants::NO_NODE_FACTS_INFO;
    }
    if (noRelationFacts) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_RELATION_FACTS_INFO;
    }
    if (noWayFacts) {
      oss << "\n" << prefix << osm2rdf::config::constants::NO_WAY_FACTS_INFO;
    } else {
      if (addWayMetadata) {
        oss << "\n"
            << prefix << osm2rdf::config::constants::ADD_WAY_METADATA_INFO;
      }
      if (!addMemberTriples) {
        oss << "\n"
            << prefix << osm2rdf::config::constants::NO_MEMBER_TRIPLES_INFO;
      }
      if (addWayNodeSpatialMetadata) {
        oss << "\n"
            << prefix
            << osm2rdf::config::constants::ADD_WAY_NODE_SPATIAL_METADATA_INFO;
      }
      if (addAreaWayLinestrings) {
        oss << "\n"
            << prefix
            << osm2rdf::config::constants::ADD_AREA_WAY_LINESTRINGS_INFO;
      }
    }
    if (!addUntaggedNodes) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_UNTAGGED_NODES_INFO;
    }
    if (!addUntaggedWays) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_UNTAGGED_WAYS_INFO;
    }
    if (!addUntaggedRelations) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_UNTAGGED_RELATIONS_INFO;
    }
    if (!addUntaggedAreas) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_UNTAGGED_AREAS_INFO;
    }
    if (addSpatialRelsForUntaggedNodes) {
      oss << "\n"
          << prefix
          << osm2rdf::config::constants::UNTAGGED_NODES_SPATIAL_RELS_INFO;
    }
    if (simplifyWKT > 0) {
      oss << "\n" << prefix << osm2rdf::config::constants::SIMPLIFY_WKT_INFO;
      oss << "\n"
          << prefix << osm2rdf::config::constants::SIMPLIFY_WKT_DEVIATION_INFO
          << std::to_string(wktDeviation);
    }
    if (skipWikiLinks) {
      oss << "\n" << prefix << osm2rdf::config::constants::SKIP_WIKI_LINKS_INFO;
    }
    oss << "\n"
        << prefix << osm2rdf::config::constants::WKT_PRECISION_INFO
        << std::to_string(wktPrecision);
    if (!semicolonTagKeys.empty()) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::SEMICOLON_TAG_KEYS_INFO;
      std::vector<std::string> keys;
      keys.reserve(semicolonTagKeys.size());
      for (const auto& key : semicolonTagKeys) {
        keys.push_back(key);
      }
      std::sort(keys.begin(), keys.end());
      for (const auto& key : keys) {
        oss << "\n" << prefix << prefix << key;
      }
    }
  }
  oss << "\n" << prefix << osm2rdf::config::constants::SECTION_CONTAINS;
  std::string modeStrings[2] = {"none", "full"};

  oss << "\n"
      << prefix << osm2rdf::config::constants::OGC_GEO_TRIPLES_INFO << ": "
      << (modeStrings[ogcGeoTriplesMode]);

  if (ogcGeoTriplesMode) {
    if (noAreaGeometricRelations) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_AREA_GEOM_RELATIONS_INFO;
    }
    if (noNodeGeometricRelations) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_NODE_GEOM_RELATIONS_INFO;
    }
    if (noRelationGeometricRelations) {
      oss << "\n"
          << prefix
          << osm2rdf::config::constants::NO_RELATION_GEOM_RELATIONS_INFO;
    }
    if (noWayGeometricRelations) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::NO_WAY_GEOM_RELATIONS_INFO;
    }
    if (simplifyGeometries > 0) {
      oss << "\n"
          << prefix << osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_INFO
          << std::to_string(simplifyGeometries);
    }
  }
  oss << "\n" << prefix << osm2rdf::config::constants::SECTION_MISCELLANEOUS;
  oss << "\n" << prefix << "Num Threads: " << numThreads;

  if (noBlankNodes) {
    oss << "\n"
        << prefix
        << osm2rdf::config::constants::BLANK_NODES_INFO;
  }

  if (!storeLocations.empty()) {
    oss << "\n"
        << prefix << osm2rdf::config::constants::STORE_LOCATIONS_INFO << " "
        << storeLocations;
  }

  if (writeRDFStatistics) {
    oss << "\n"
        << prefix << osm2rdf::config::constants::WRITE_RDF_STATISTICS_INFO;
  }
  if (outputKeepFiles) {
    oss << "\n"
        << prefix << osm2rdf::config::constants::OUTPUT_KEEP_FILES_OPTION_INFO;
  }
#if defined(_OPENMP)
  oss << "\n" << prefix << osm2rdf::config::constants::SECTION_OPENMP;
  oss << "\n" << prefix << "Max Threads: " << omp_get_max_threads();
#endif
  return oss.str();
}

// ____________________________________________________________________________
void osm2rdf::config::Config::fromArgs(int argc, char** argv) {
  popl::OptionParser parser("Allowed options");

  auto helpOp =
      parser.add<popl::Switch>(osm2rdf::config::constants::HELP_OPTION_SHORT,
                               osm2rdf::config::constants::HELP_OPTION_LONG,
                               osm2rdf::config::constants::HELP_OPTION_HELP);

  auto storeLocationsOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::STORE_LOCATIONS_SHORT,
          osm2rdf::config::constants::STORE_LOCATIONS_LONG,
          osm2rdf::config::constants::STORE_LOCATIONS_HELP, "mem-flex");

  auto noAreasOp = parser.add<popl::Switch, popl::Attribute::advanced>(
      osm2rdf::config::constants::NO_AREA_OPTION_SHORT,
      osm2rdf::config::constants::NO_AREA_OPTION_LONG,
      osm2rdf::config::constants::NO_AREA_OPTION_HELP);
  auto noNodesOp = parser.add<popl::Switch, popl::Attribute::advanced>(
      osm2rdf::config::constants::NO_NODE_OPTION_SHORT,
      osm2rdf::config::constants::NO_NODE_OPTION_LONG,
      osm2rdf::config::constants::NO_NODE_OPTION_HELP);
  auto noRelationsOp = parser.add<popl::Switch, popl::Attribute::advanced>(
      osm2rdf::config::constants::NO_RELATION_OPTION_SHORT,
      osm2rdf::config::constants::NO_RELATION_OPTION_LONG,
      osm2rdf::config::constants::NO_RELATION_OPTION_HELP);
  auto noWaysOp = parser.add<popl::Switch, popl::Attribute::advanced>(
      osm2rdf::config::constants::NO_WAY_OPTION_SHORT,
      osm2rdf::config::constants::NO_WAY_OPTION_LONG,
      osm2rdf::config::constants::NO_WAY_OPTION_HELP);

  auto noFactsOp = parser.add<popl::Switch, popl::Attribute::advanced>(
      osm2rdf::config::constants::NO_FACTS_OPTION_SHORT,
      osm2rdf::config::constants::NO_FACTS_OPTION_LONG,
      osm2rdf::config::constants::NO_FACTS_OPTION_HELP);
  auto noAreaFactsOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_AREA_FACTS_OPTION_SHORT,
      osm2rdf::config::constants::NO_AREA_FACTS_OPTION_LONG,
      osm2rdf::config::constants::NO_AREA_FACTS_OPTION_HELP);
  auto noNodeFactsOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_NODE_FACTS_OPTION_SHORT,
      osm2rdf::config::constants::NO_NODE_FACTS_OPTION_LONG,
      osm2rdf::config::constants::NO_NODE_FACTS_OPTION_HELP);
  auto noRelationFactsOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_RELATION_FACTS_OPTION_SHORT,
      osm2rdf::config::constants::NO_RELATION_FACTS_OPTION_LONG,
      osm2rdf::config::constants::NO_RELATION_FACTS_OPTION_HELP);
  auto noWayFactsOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_WAY_FACTS_OPTION_SHORT,
      osm2rdf::config::constants::NO_WAY_FACTS_OPTION_LONG,
      osm2rdf::config::constants::NO_WAY_FACTS_OPTION_HELP);

  auto sourceDatasetOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::SOURCE_DATASET_OPTION_SHORT,
          osm2rdf::config::constants::SOURCE_DATASET_OPTION_LONG,
          osm2rdf::config::constants::SOURCE_DATASET_OPTION_HELP, "OSM");

  auto noAreaGeometricRelationsOp =
      parser.add<popl::Switch, popl::Attribute::expert>(
          osm2rdf::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_SHORT,
          osm2rdf::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_LONG,
          osm2rdf::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_HELP);
  auto noNodeGeometricRelationsOp =
      parser.add<popl::Switch, popl::Attribute::expert>(
          osm2rdf::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_SHORT,
          osm2rdf::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_LONG,
          osm2rdf::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_HELP);
  auto noRelationGeometricRelationsOp =
      parser.add<popl::Switch, popl::Attribute::expert>(
          osm2rdf::config::constants::NO_RELATION_GEOM_RELATIONS_OPTION_SHORT,
          osm2rdf::config::constants::NO_RELATION_GEOM_RELATIONS_OPTION_LONG,
          osm2rdf::config::constants::NO_RELATION_GEOM_RELATIONS_OPTION_HELP);
  auto noWayGeometricRelationsOp =
      parser.add<popl::Switch, popl::Attribute::expert>(
          osm2rdf::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_SHORT,
          osm2rdf::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_LONG,
          osm2rdf::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_HELP);
  auto ogcGeoTriplesModeOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::OGC_GEO_TRIPLES_OPTION_SHORT,
          osm2rdf::config::constants::OGC_GEO_TRIPLES_OPTION_LONG,
          osm2rdf::config::constants::OGC_GEO_TRIPLES_OPTION_HELP, "full");

  auto noAddCentroidsOp = parser.add<popl::Switch, popl::Attribute::advanced>(
      osm2rdf::config::constants::NO_ADD_CENTROIDS_OPTION_SHORT,
      osm2rdf::config::constants::NO_ADD_CENTROIDS_OPTION_LONG,
      osm2rdf::config::constants::NO_ADD_CENTROIDS_OPTION_HELP);

  auto addAreaWayLinestringsOp =
      parser.add<popl::Switch, popl::Attribute::expert>(
          osm2rdf::config::constants::ADD_AREA_WAY_LINESTRINGS_OPTION_SHORT,
          osm2rdf::config::constants::ADD_AREA_WAY_LINESTRINGS_OPTION_LONG,
          osm2rdf::config::constants::ADD_AREA_WAY_LINESTRINGS_OPTION_HELP);

  auto untaggedNodesSpatialRelsOp = parser.add<popl::Switch,
                                                 popl::Attribute::expert>(
      osm2rdf::config::constants::UNTAGGED_NODES_SPATIAL_RELS_OPTION_SHORT,
      osm2rdf::config::constants::UNTAGGED_NODES_SPATIAL_RELS_OPTION_LONG,
      osm2rdf::config::constants::UNTAGGED_NODES_SPATIAL_RELS_OPTION_HELP);

  auto noUntaggedNodesOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_UNTAGGED_NODES_OPTION_SHORT,
      osm2rdf::config::constants::NO_UNTAGGED_NODES_OPTION_LONG,
      osm2rdf::config::constants::NO_UNTAGGED_NODES_OPTION_HELP);

  auto noUntaggedWaysOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_UNTAGGED_WAYS_OPTION_SHORT,
      osm2rdf::config::constants::NO_UNTAGGED_WAYS_OPTION_LONG,
      osm2rdf::config::constants::NO_UNTAGGED_WAYS_OPTION_HELP);

  auto noUntaggedRelationsOp =
      parser.add<popl::Switch, popl::Attribute::expert>(
          osm2rdf::config::constants::NO_UNTAGGED_RELATIONS_OPTION_SHORT,
          osm2rdf::config::constants::NO_UNTAGGED_RELATIONS_OPTION_LONG,
          osm2rdf::config::constants::NO_UNTAGGED_RELATIONS_OPTION_HELP);

  auto noUntaggedAreasOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::NO_UNTAGGED_AREAS_OPTION_SHORT,
      osm2rdf::config::constants::NO_UNTAGGED_AREAS_OPTION_LONG,
      osm2rdf::config::constants::NO_UNTAGGED_AREAS_OPTION_HELP);

  auto noOsmMetadataOp = parser.add<popl::Switch>(
      osm2rdf::config::constants::NO_OSM_METADATA_OPTION_SHORT,
      osm2rdf::config::constants::NO_OSM_METADATA_OPTION_LONG,
      osm2rdf::config::constants::NO_OSM_METADATA_OPTION_HELP);
  auto addWayMetadataOp = parser.add<popl::Switch>(
      osm2rdf::config::constants::ADD_WAY_METADATA_OPTION_SHORT,
      osm2rdf::config::constants::ADD_WAY_METADATA_OPTION_LONG,
      osm2rdf::config::constants::ADD_WAY_METADATA_OPTION_HELP);
  auto noMemberTriplesOp = parser.add<popl::Switch>(
      osm2rdf::config::constants::NO_MEMBER_TRIPLES_OPTION_SHORT,
      osm2rdf::config::constants::NO_MEMBER_TRIPLES_OPTION_LONG,
      osm2rdf::config::constants::NO_MEMBER_TRIPLES_OPTION_HELP);
  auto addWayNodeSpatialMetadataOp = parser.add<popl::Switch>(
      osm2rdf::config::constants::ADD_WAY_NODE_SPATIAL_METADATA_OPTION_SHORT,
      osm2rdf::config::constants::ADD_WAY_NODE_SPATIAL_METADATA_OPTION_LONG,
      osm2rdf::config::constants::ADD_WAY_NODE_SPATIAL_METADATA_OPTION_HELP);
  auto skipWikiLinksOp = parser.add<popl::Switch>(
      osm2rdf::config::constants::SKIP_WIKI_LINKS_OPTION_SHORT,
      osm2rdf::config::constants::SKIP_WIKI_LINKS_OPTION_LONG,
      osm2rdf::config::constants::SKIP_WIKI_LINKS_OPTION_HELP);

  auto auxGeoFilesOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::AUX_GEO_FILES_OPTION_SHORT,
          osm2rdf::config::constants::AUX_GEO_FILES_OPTION_LONG,
          osm2rdf::config::constants::AUX_GEO_FILES_OPTION_HELP);

  auto numThreadsOp = parser.add<popl::Value<int>, popl::Attribute::advanced>(
      osm2rdf::config::constants::NUM_THREADS_OPTION_SHORT,
      osm2rdf::config::constants::NUM_THREADS_OPTION_LONG,
      osm2rdf::config::constants::NUM_THREADS_OPTION_HELP, numThreads);

  auto semicolonTagKeysOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::SEMICOLON_TAG_KEYS_OPTION_SHORT,
          osm2rdf::config::constants::SEMICOLON_TAG_KEYS_OPTION_LONG,
          osm2rdf::config::constants::SEMICOLON_TAG_KEYS_OPTION_HELP);

  auto simplifyGeometriesOp =
      parser.add<popl::Value<double>, popl::Attribute::expert>(
          osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_OPTION_SHORT,
          osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_OPTION_LONG,
          osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_OPTION_HELP,
          simplifyGeometries);

  auto simplifyWKTOp =
      parser.add<popl::Value<uint16_t>, popl::Attribute::advanced>(
          osm2rdf::config::constants::SIMPLIFY_WKT_OPTION_SHORT,
          osm2rdf::config::constants::SIMPLIFY_WKT_OPTION_LONG,
          osm2rdf::config::constants::SIMPLIFY_WKT_OPTION_HELP, simplifyWKT);
  auto wktDeviationOp =
      parser.add<popl::Value<double>, popl::Attribute::expert>(
          osm2rdf::config::constants::SIMPLIFY_WKT_DEVIATION_OPTION_SHORT,
          osm2rdf::config::constants::SIMPLIFY_WKT_DEVIATION_OPTION_LONG,
          osm2rdf::config::constants::SIMPLIFY_WKT_DEVIATION_OPTION_HELP,
          wktDeviation);
  auto wktPrecisionOp =
      parser.add<popl::Value<uint16_t>, popl::Attribute::advanced>(
          osm2rdf::config::constants::WKT_PRECISION_OPTION_SHORT,
          osm2rdf::config::constants::WKT_PRECISION_OPTION_LONG,
          osm2rdf::config::constants::WKT_PRECISION_OPTION_HELP, wktPrecision);

  auto noBlankNodesOp =
      parser.add<popl::Switch, popl::Attribute::advanced>(
          osm2rdf::config::constants::BLANK_NODES_OPTION_SHORT,
          osm2rdf::config::constants::BLANK_NODES_OPTION_LONG,
          osm2rdf::config::constants::BLANK_NODES_OPTION_HELP);

  auto writeRDFStatisticsOp =
      parser.add<popl::Switch, popl::Attribute::advanced>(
          osm2rdf::config::constants::WRITE_RDF_STATISTICS_OPTION_SHORT,
          osm2rdf::config::constants::WRITE_RDF_STATISTICS_OPTION_LONG,
          osm2rdf::config::constants::WRITE_RDF_STATISTICS_OPTION_HELP);

  auto outputOp = parser.add<popl::Value<std::string>>(
      osm2rdf::config::constants::OUTPUT_OPTION_SHORT,
      osm2rdf::config::constants::OUTPUT_OPTION_LONG,
      osm2rdf::config::constants::OUTPUT_OPTION_HELP, "");
  auto outputFormatOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::OUTPUT_FORMAT_OPTION_SHORT,
          osm2rdf::config::constants::OUTPUT_FORMAT_OPTION_LONG,
          osm2rdf::config::constants::OUTPUT_FORMAT_OPTION_HELP, outputFormat);
  auto outputKeepFilesOp = parser.add<popl::Switch, popl::Attribute::expert>(
      osm2rdf::config::constants::OUTPUT_KEEP_FILES_OPTION_SHORT,
      osm2rdf::config::constants::OUTPUT_KEEP_FILES_OPTION_LONG,
      osm2rdf::config::constants::OUTPUT_KEEP_FILES_OPTION_HELP);
  auto outputCompressOp =
      parser.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2rdf::config::constants::OUTPUT_COMPRESS_OPTION_SHORT,
          osm2rdf::config::constants::OUTPUT_COMPRESS_OPTION_LONG,
          osm2rdf::config::constants::OUTPUT_COMPRESS_OPTION_HELP, "bz2");
  auto cacheOp = parser.add<popl::Value<std::string>>(
      osm2rdf::config::constants::CACHE_OPTION_SHORT,
      osm2rdf::config::constants::CACHE_OPTION_LONG,
      osm2rdf::config::constants::CACHE_OPTION_HELP, cache);

  try {
    parser.parse(argc, argv);

    if (helpOp->count() > 0) {
      if (helpOp->count() == 1) {
        std::cerr << parser << "\n";
      } else if (helpOp->count() == 2) {
        std::cerr << parser.help(popl::Attribute::advanced) << "\n";
      } else {
        std::cerr << parser.help(popl::Attribute::expert) << "\n";
      }
      exit(osm2rdf::config::ExitCode::SUCCESS);
    }

    if (!parser.unknown_options().empty()) {
      std::cerr << "Unknown argument(s) specified:\n";
      for (const auto& option : parser.unknown_options()) {
        std::cerr << option << "\n";
      }
      std::cerr << "\n" << parser.help() << "\n";
      exit(osm2rdf::config::ExitCode::UNKNOWN_ARGUMENT);
    }

    // Skip passes
    noFacts = noFactsOp->is_set();

    if (storeLocationsOp->is_set()) {
      storeLocations = storeLocationsOp->value();
    }

    // Select types to dump
    noAreaFacts = noAreaFactsOp->is_set();
    noNodeFacts = noNodeFactsOp->is_set();
    noRelationFacts = noRelationFactsOp->is_set();
    noWayFacts = noWayFactsOp->is_set();

    noAreaGeometricRelations = noAreaGeometricRelationsOp->is_set();
    noNodeGeometricRelations = noNodeGeometricRelationsOp->is_set();
    noRelationGeometricRelations = noRelationGeometricRelationsOp->is_set();
    noWayGeometricRelations = noWayGeometricRelationsOp->is_set();

    if (ogcGeoTriplesModeOp->is_set()) {
      if (ogcGeoTriplesModeOp->value() == "none") {
        ogcGeoTriplesMode = none;
      } else if (ogcGeoTriplesModeOp->value() == "full") {
        ogcGeoTriplesMode = full;
      } else {
        throw popl::invalid_option(
            ogcGeoTriplesModeOp.get(),
            popl::invalid_option::Error::invalid_argument,
            popl::OptionName::long_name, ogcGeoTriplesModeOp->value(), "");
      }
    }

    noGeometricRelations = ogcGeoTriplesMode == none;

    noAreaFacts |= noAreasOp->is_set();
    noAreaGeometricRelations |= noAreasOp->is_set();
    noNodeFacts |= noNodesOp->is_set();
    noNodeGeometricRelations |= noNodesOp->is_set();
    noRelationFacts |= noRelationsOp->is_set();
    noRelationGeometricRelations |= noWaysOp->is_set();
    noWayFacts |= noWaysOp->is_set();
    noWayGeometricRelations |= noWaysOp->is_set();

    // Dataset selection
    if (sourceDatasetOp->is_set()) {
      if (sourceDatasetOp->value() == "OSM") {
        sourceDataset = OSM;
      } else if (sourceDatasetOp->value() == "OHM") {
        sourceDataset = OHM;
      } else {
        throw popl::invalid_option(
            sourceDatasetOp.get(),
            popl::invalid_option::Error::invalid_argument,
            popl::OptionName::long_name, sourceDatasetOp->value(), "");
      }
    }

    // Select amount to dump
    addAreaWayLinestrings = addAreaWayLinestringsOp->is_set();
    addCentroids = !noAddCentroidsOp->is_set();
    addWayMetadata = addWayMetadataOp->is_set();
    addOsmMetadata = !noOsmMetadataOp->is_set();
    addMemberTriples = !noMemberTriplesOp->is_set();
    addWayNodeSpatialMetadata = addWayNodeSpatialMetadataOp->is_set();
    skipWikiLinks = skipWikiLinksOp->is_set();
    simplifyGeometries = simplifyGeometriesOp->value();
    simplifyWKT = simplifyWKTOp->value();
    wktDeviation = wktDeviationOp->value();
    wktPrecision = wktPrecisionOp->value();

    addSpatialRelsForUntaggedNodes = untaggedNodesSpatialRelsOp->is_set();

    noBlankNodes = noBlankNodesOp->is_set();

    addUntaggedNodes = !noUntaggedNodesOp->is_set();
    addUntaggedWays = !noUntaggedWaysOp->is_set();
    addUntaggedRelations = !noUntaggedRelationsOp->is_set();
    addUntaggedAreas = !noUntaggedAreasOp->is_set();

    addMemberTriples |= addWayNodeSpatialMetadata;

    if (semicolonTagKeysOp->is_set()) {
      for (size_t i = 0; i < semicolonTagKeysOp->count(); ++i) {
        semicolonTagKeys.insert(semicolonTagKeysOp->value(i));
      }
    }
    if (auxGeoFilesOp->is_set()) {
      for (size_t i = 0; i < auxGeoFilesOp->count(); ++i) {
        auxGeoFiles.push_back(auxGeoFilesOp->value(i));
      }
    }

    if (numThreadsOp->is_set()) numThreads = numThreadsOp->value();

    writeRDFStatistics = writeRDFStatisticsOp->is_set();

    // Output
    output = outputOp->value();
    outputFormat = outputFormatOp->value();
    if (outputCompressOp->value() == "none") {
      outputCompress = NONE;
    } else if (outputCompressOp->value() == "gz") {
      outputCompress = GZ;
    } else if (outputCompressOp->value() == "bz2") {
      outputCompress = BZ2;
    } else {
      throw popl::invalid_option(
          outputCompressOp.get(), popl::invalid_option::Error::invalid_argument,
          popl::OptionName::long_name, outputCompressOp->value(), "");
    }

    outputKeepFiles = outputKeepFilesOp->is_set();
    if (output.empty()) {
      outputCompress = NONE;
      mergeOutput = util::OutputMergeMode::NONE;
    }

    // Paths for statistic files
    rdfStatisticsPath = std::filesystem::path(output);
    rdfStatisticsPath += osm2rdf::config::constants::STATS_EXTENSION;
    rdfStatisticsPath += osm2rdf::config::constants::JSON_EXTENSION;

    // Mark compressed output
    if (outputCompress == BZ2 && !output.empty() &&
        output.extension() != osm2rdf::config::constants::BZIP2_EXTENSION) {
      output += osm2rdf::config::constants::BZIP2_EXTENSION;
    }

    if (outputCompress == GZ && !output.empty() &&
        output.extension() != osm2rdf::config::constants::GZ_EXTENSION) {
      output += osm2rdf::config::constants::GZ_EXTENSION;
    }

    // osmium location cache
    cache = std::filesystem::absolute(cacheOp->value()).string();

    // Check cache location
    if (!std::filesystem::exists(cache)) {
      std::cerr << "Cache location does not exist: " << cache << "\n"
                << parser.help() << "\n";
      exit(osm2rdf::config::ExitCode::CACHE_NOT_EXISTS);
    }
    if (!std::filesystem::is_directory(cache)) {
      std::cerr << "Cache location not a directory: " << cache << "\n"
                << parser.help() << "\n";
      exit(osm2rdf::config::ExitCode::CACHE_NOT_DIRECTORY);
    }

    // Handle input
    if (parser.non_option_args().size() != 1) {
      std::cerr << "No input specified!\n" << parser.help() << "\n";
      exit(osm2rdf::config::ExitCode::INPUT_MISSING);
    }
    input = parser.non_option_args()[0];
    if (!std::filesystem::exists(input)) {
      std::cerr << "Input does not exist: " << input << "\n"
                << parser.help() << "\n";
      exit(osm2rdf::config::ExitCode::INPUT_NOT_EXISTS);
    }
    if (std::filesystem::is_directory(input)) {
      std::cerr << "Input is a directory: " << input << "\n"
                << parser.help() << "\n";
      exit(osm2rdf::config::ExitCode::INPUT_IS_DIRECTORY);
    }
  } catch (const popl::invalid_option& e) {
    std::cerr << "Invalid Option Exception: " << e.what() << "\n";
    std::cerr << "error:  ";
    if (e.error() == popl::invalid_option::Error::missing_argument) {
      std::cerr << "missing_argument\n";
    } else if (e.error() == popl::invalid_option::Error::invalid_argument) {
      std::cerr << "invalid_argument\n";
    } else if (e.error() == popl::invalid_option::Error::too_many_arguments) {
      std::cerr << "too_many_arguments\n";
    } else if (e.error() == popl::invalid_option::Error::missing_option) {
      std::cerr << "missing_option\n";
    }

    if (e.error() == popl::invalid_option::Error::missing_option) {
      std::string option_name(
          e.option()->name(popl::OptionName::short_name, true));
      if (option_name.empty()) {
        option_name = e.option()->name(popl::OptionName::long_name, true);
      }
      std::cerr << "option: " << option_name << "\n";
    } else {
      std::cerr << "option: " << e.option()->name(e.what_name()) << "\n";
      std::cerr << "value:  " << e.value() << "\n";
    }
    exit(osm2rdf::config::ExitCode::FAILURE);
  }
}

// ____________________________________________________________________________
std::filesystem::path osm2rdf::config::Config::getTempPath(
    const std::string& path, const std::string& suffix) const {
  std::filesystem::path resultPath{cache};
  resultPath /= path + "-" + suffix;
  return std::filesystem::absolute(resultPath);
}
