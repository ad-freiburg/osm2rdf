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

#include "osm2ttl/config/Config.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "omp.h"
#include "osm2ttl/config/Constants.h"
#include "osm2ttl/config/ExitCode.h"
#include "popl.hpp"

// ____________________________________________________________________________
std::string osm2ttl::config::Config::getInfo(std::string_view prefix) const {
  std::ostringstream oss;
  oss << prefix << osm2ttl::config::constants::HEADER;
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_IO;
  oss << "\n"
      << prefix << osm2ttl::config::constants::INPUT_INFO << "         "
      << input;
  oss << "\n"
      << prefix << osm2ttl::config::constants::OUTPUT_INFO << "        "
      << output;
  oss << "\n"
      << prefix << osm2ttl::config::constants::OUTPUT_FORMAT_INFO << " "
      << outputFormat;
  oss << "\n"
      << prefix << osm2ttl::config::constants::CACHE_INFO << "         "
      << cache;
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_FACTS;
  oss << "\n"
      << prefix << osm2ttl::config::constants::OSM2TTL_PREFIX_INFO << ": "
      << osm2ttlPrefix;
  if (noFacts) {
    oss << "\n" << prefix << osm2ttl::config::constants::NO_FACTS_INFO;
  } else {
    if (adminRelationsOnly) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::ADMIN_RELATIONS_ONLY_INFO;
    }
    if (noAreaFacts) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_AREA_FACTS_INFO;
    } else {
      if (addAreaEnvelope) {
        oss << "\n"
            << prefix << osm2ttl::config::constants::ADD_AREA_ENVELOPE_INFO;
      }
      if (addAreaEnvelopeRatio) {
        oss << "\n"
            << prefix
            << osm2ttl::config::constants::ADD_AREA_ENVELOPE_RATIO_INFO;
      }
    }
    if (noNodeFacts) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_NODE_FACTS_INFO;
    } else {
      if (addNodeEnvelope) {
        oss << "\n"
            << prefix << osm2ttl::config::constants::ADD_NODE_ENVELOPE_INFO;
      }
    }
    if (noRelationFacts) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_RELATION_FACTS_INFO;
    }
    if (noWayFacts) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_WAY_FACTS_INFO;
    } else {
      if (addWayEnvelope) {
        oss << "\n"
            << prefix << osm2ttl::config::constants::ADD_WAY_ENVELOPE_INFO;
      }
      if (addWayMetadata) {
        oss << "\n"
            << prefix << osm2ttl::config::constants::ADD_WAY_METADATA_INFO;
      }
      if (addWayNodeOrder) {
        oss << "\n"
            << prefix << osm2ttl::config::constants::ADD_WAY_NODE_ORDER_INFO;
      }
    }
    if (simplifyWKT > 0) {
      oss << "\n" << prefix << osm2ttl::config::constants::SIMPLIFY_WKT_INFO;
      oss << "\n"
          << prefix << osm2ttl::config::constants::SIMPLIFY_WKT_DEVIATION_INFO
          << std::to_string(wktDeviation);
    }
    if (skipWikiLinks) {
      oss << "\n" << prefix << osm2ttl::config::constants::SKIP_WIKI_LINKS_INFO;
    }
    oss << "\n"
        << prefix << osm2ttl::config::constants::WKT_PRECISION_INFO
        << std::to_string(wktPrecision);
    if (!semicolonTagKeys.empty()) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::SEMICOLON_TAG_KEYS_INFO;
      std::vector<std::string> keys;
      keys.reserve(semicolonTagKeys.size());
      for (const auto &key : semicolonTagKeys) {
        keys.push_back(key);
      }
      std::sort(keys.begin(), keys.end());
      for (const auto &key : keys) {
        oss << "\n" << prefix << prefix << key;
      }
    }
  }
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_CONTAINS;
  if (noGeometricRelations) {
    oss << "\n" << prefix << osm2ttl::config::constants::NO_GEOM_RELATIONS_INFO;
  } else {
    if (adminRelationsOnly) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::ADMIN_RELATIONS_ONLY_INFO;
    }
    if (noAreaGeometricRelations) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_AREA_GEOM_RELATIONS_INFO;
    }
    if (noNodeGeometricRelations) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_NODE_GEOM_RELATIONS_INFO;
    }
    if (noWayGeometricRelations) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_WAY_GEOM_RELATIONS_INFO;
    }
    if (simplifyGeometries > 0) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::SIMPLIFY_GEOMETRIES_INFO
          << std::to_string(simplifyGeometries);
    }
  }
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_MISCELLANEOUS;
  if (minimalAreaEnvelopeRatio > 0) {
    oss << "\n"
        << prefix
        << osm2ttl::config::constants::MINIMAL_AREA_ENVELOPE_RATIO_INFO
        << std::to_string(minimalAreaEnvelopeRatio);
  }
  if (writeDAGDotFiles) {
    oss << "\n"
        << prefix << osm2ttl::config::constants::WRITE_DAG_DOT_FILES_INFO;
  }
  if (storeLocationsOnDisk) {
    oss << "\n"
        << prefix << osm2ttl::config::constants::STORE_LOCATIONS_ON_DISK_INFO;
  }
  if (writeGeometricRelationStatistics) {
#ifdef ENABLE_GEOMETRY_STATISTIC
    oss << "\n"
        << prefix
        << osm2ttl::config::constants::WRITE_GEOM_RELATION_STATISTICS_INFO;
#else
    oss << "\n"
        << prefix
        << osm2ttl::config::constants::
               WRITE_GEOM_RELATION_STATISTICS_INFO_DISABLED;
#endif
  }
#if defined(_OPENMP)
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_OPENMP;
  oss << "\n" << prefix << "Max Threads: " << omp_get_max_threads();
#endif
  return oss.str();
}

// ____________________________________________________________________________
void osm2ttl::config::Config::fromArgs(int argc, char** argv) {
  popl::OptionParser op("Allowed options");

  auto helpOp =
      op.add<popl::Switch>(osm2ttl::config::constants::HELP_OPTION_SHORT,
                           osm2ttl::config::constants::HELP_OPTION_LONG,
                           osm2ttl::config::constants::HELP_OPTION_HELP);

  auto storeLocationsOnDiskOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::STORE_LOCATIONS_ON_DISK_SHORT,
      osm2ttl::config::constants::STORE_LOCATIONS_ON_DISK_LONG,
      osm2ttl::config::constants::STORE_LOCATIONS_ON_DISK_HELP);

  auto noAreasOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::NO_AREA_OPTION_SHORT,
      osm2ttl::config::constants::NO_AREA_OPTION_LONG,
      osm2ttl::config::constants::NO_AREA_OPTION_HELP);
  auto noNodesOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::NO_NODE_OPTION_SHORT,
      osm2ttl::config::constants::NO_NODE_OPTION_LONG,
      osm2ttl::config::constants::NO_NODE_OPTION_HELP);
  auto noRelationsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::NO_RELATION_OPTION_SHORT,
      osm2ttl::config::constants::NO_RELATION_OPTION_LONG,
      osm2ttl::config::constants::NO_RELATION_OPTION_HELP);
  auto noWaysOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::NO_WAY_OPTION_SHORT,
      osm2ttl::config::constants::NO_WAY_OPTION_LONG,
      osm2ttl::config::constants::NO_WAY_OPTION_HELP);

  auto noFactsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::NO_FACTS_OPTION_SHORT,
      osm2ttl::config::constants::NO_FACTS_OPTION_LONG,
      osm2ttl::config::constants::NO_FACTS_OPTION_HELP);
  auto noAreaFactsOp = op.add<popl::Switch, popl::Attribute::expert>(
      osm2ttl::config::constants::NO_AREA_FACTS_OPTION_SHORT,
      osm2ttl::config::constants::NO_AREA_FACTS_OPTION_LONG,
      osm2ttl::config::constants::NO_AREA_FACTS_OPTION_HELP);
  auto noNodeFactsOp = op.add<popl::Switch, popl::Attribute::expert>(
      osm2ttl::config::constants::NO_NODE_FACTS_OPTION_SHORT,
      osm2ttl::config::constants::NO_NODE_FACTS_OPTION_LONG,
      osm2ttl::config::constants::NO_NODE_FACTS_OPTION_HELP);
  auto noRelationFactsOp = op.add<popl::Switch, popl::Attribute::expert>(
      osm2ttl::config::constants::NO_RELATION_FACTS_OPTION_SHORT,
      osm2ttl::config::constants::NO_RELATION_FACTS_OPTION_LONG,
      osm2ttl::config::constants::NO_RELATION_FACTS_OPTION_HELP);
  auto noWayFactsOp = op.add<popl::Switch, popl::Attribute::expert>(
      osm2ttl::config::constants::NO_WAY_FACTS_OPTION_SHORT,
      osm2ttl::config::constants::NO_WAY_FACTS_OPTION_LONG,
      osm2ttl::config::constants::NO_WAY_FACTS_OPTION_HELP);

  auto noGeometricRelationsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::NO_GEOM_RELATIONS_OPTION_SHORT,
      osm2ttl::config::constants::NO_GEOM_RELATIONS_OPTION_LONG,
      osm2ttl::config::constants::NO_GEOM_RELATIONS_OPTION_HELP);
  auto noAreaGeometricRelationsOp =
      op.add<popl::Switch, popl::Attribute::expert>(
          osm2ttl::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_SHORT,
          osm2ttl::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_LONG,
          osm2ttl::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_HELP);
  auto noNodeGeometricRelationsOp =
      op.add<popl::Switch, popl::Attribute::expert>(
          osm2ttl::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_SHORT,
          osm2ttl::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_LONG,
          osm2ttl::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_HELP);
  auto noWayGeometricRelationsOp =
      op.add<popl::Switch, popl::Attribute::expert>(
          osm2ttl::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_SHORT,
          osm2ttl::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_LONG,
          osm2ttl::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_HELP);

  auto addAreaEnvelopeOp = op.add<popl::Switch>(
      osm2ttl::config::constants::ADD_AREA_ENVELOPE_OPTION_SHORT,
      osm2ttl::config::constants::ADD_AREA_ENVELOPE_OPTION_LONG,
      osm2ttl::config::constants::ADD_AREA_ENVELOPE_OPTION_HELP);
  auto addAreaEnvelopeRatioOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::ADD_AREA_ENVELOPE_RATIO_OPTION_SHORT,
      osm2ttl::config::constants::ADD_AREA_ENVELOPE_RATIO_OPTION_LONG,
      osm2ttl::config::constants::ADD_AREA_ENVELOPE_RATIO_OPTION_HELP);
  auto addNodeEnvelopeOp = op.add<popl::Switch>(
      osm2ttl::config::constants::ADD_NODE_ENVELOPE_OPTION_SHORT,
      osm2ttl::config::constants::ADD_NODE_ENVELOPE_OPTION_LONG,
      osm2ttl::config::constants::ADD_NODE_ENVELOPE_OPTION_HELP);
  auto addWayEnvelopeOp = op.add<popl::Switch>(
      osm2ttl::config::constants::ADD_WAY_ENVELOPE_OPTION_SHORT,
      osm2ttl::config::constants::ADD_WAY_ENVELOPE_OPTION_LONG,
      osm2ttl::config::constants::ADD_WAY_ENVELOPE_OPTION_HELP);
  auto addWayMetaDataOp = op.add<popl::Switch>(
      osm2ttl::config::constants::ADD_WAY_METADATA_OPTION_SHORT,
      osm2ttl::config::constants::ADD_WAY_METADATA_OPTION_LONG,
      osm2ttl::config::constants::ADD_WAY_METADATA_OPTION_HELP);
  auto addWayNodeOrderOp = op.add<popl::Switch>(
      osm2ttl::config::constants::ADD_WAY_NODE_ORDER_OPTION_SHORT,
      osm2ttl::config::constants::ADD_WAY_NODE_ORDER_OPTION_LONG,
      osm2ttl::config::constants::ADD_WAY_NODE_ORDER_OPTION_HELP);
  auto adminRelationsOnlyOp = op.add<popl::Switch>(
      osm2ttl::config::constants::ADMIN_RELATIONS_ONLY_OPTION_SHORT,
      osm2ttl::config::constants::ADMIN_RELATIONS_ONLY_OPTION_LONG,
      osm2ttl::config::constants::ADMIN_RELATIONS_ONLY_OPTION_HELP);
  auto skipWikiLinksOp = op.add<popl::Switch>(
      osm2ttl::config::constants::SKIP_WIKI_LINKS_OPTION_SHORT,
      osm2ttl::config::constants::SKIP_WIKI_LINKS_OPTION_LONG,
      osm2ttl::config::constants::SKIP_WIKI_LINKS_OPTION_HELP);

  auto semicolonTagKeysOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2ttl::config::constants::SEMICOLON_TAG_KEYS_OPTION_SHORT,
          osm2ttl::config::constants::SEMICOLON_TAG_KEYS_OPTION_LONG,
          osm2ttl::config::constants::SEMICOLON_TAG_KEYS_OPTION_HELP);
  auto osm2ttlPrefixOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2ttl::config::constants::OSM2TTL_PREFIX_OPTION_SHORT,
          osm2ttl::config::constants::OSM2TTL_PREFIX_OPTION_LONG,
          osm2ttl::config::constants::OSM2TTL_PREFIX_OPTION_HELP,
          osm2ttlPrefix);

  auto simplifyGeometriesOp =
      op.add<popl::Value<uint16_t>, popl::Attribute::expert>(
          osm2ttl::config::constants::SIMPLIFY_GEOMETRIES_OPTION_SHORT,
          osm2ttl::config::constants::SIMPLIFY_GEOMETRIES_OPTION_LONG,
          osm2ttl::config::constants::SIMPLIFY_GEOMETRIES_OPTION_HELP,
          simplifyGeometries);
  auto simplifyWKTOp = op.add<popl::Value<uint16_t>, popl::Attribute::advanced>(
      osm2ttl::config::constants::SIMPLIFY_WKT_OPTION_SHORT,
      osm2ttl::config::constants::SIMPLIFY_WKT_OPTION_LONG,
      osm2ttl::config::constants::SIMPLIFY_WKT_OPTION_HELP, simplifyWKT);
  auto wktDeviationOp = op.add<popl::Value<uint16_t>, popl::Attribute::expert>(
      osm2ttl::config::constants::SIMPLIFY_WKT_DEVIATION_OPTION_SHORT,
      osm2ttl::config::constants::SIMPLIFY_WKT_DEVIATION_OPTION_LONG,
      osm2ttl::config::constants::SIMPLIFY_WKT_DEVIATION_OPTION_HELP,
      wktDeviation);
  auto wktPrecisionOp =
      op.add<popl::Value<uint16_t>, popl::Attribute::advanced>(
          osm2ttl::config::constants::WKT_PRECISION_OPTION_SHORT,
          osm2ttl::config::constants::WKT_PRECISION_OPTION_LONG,
          osm2ttl::config::constants::WKT_PRECISION_OPTION_HELP, wktPrecision);
  auto minimalAreaEnvelopeRatioOp =
      op.add<popl::Value<double>, popl::Attribute::advanced>(
          osm2ttl::config::constants::MINIMAL_AREA_ENVELOPE_RATIO_OPTION_SHORT,
          osm2ttl::config::constants::MINIMAL_AREA_ENVELOPE_RATIO_OPTION_LONG,
          osm2ttl::config::constants::MINIMAL_AREA_ENVELOPE_RATIO_OPTION_HELP,
          minimalAreaEnvelopeRatio);

  auto writeDotFilesOp = op.add<popl::Switch, popl::Attribute::expert>(
      osm2ttl::config::constants::WRITE_DAG_DOT_FILES_OPTION_SHORT,
      osm2ttl::config::constants::WRITE_DAG_DOT_FILES_OPTION_LONG,
      osm2ttl::config::constants::WRITE_DAG_DOT_FILES_OPTION_HELP);

  auto writeStatisticsOp = op.add<popl::Switch, popl::Attribute::expert>(
      osm2ttl::config::constants::WRITE_GEOM_RELATION_STATISTICS_OPTION_SHORT,
      osm2ttl::config::constants::WRITE_GEOM_RELATION_STATISTICS_OPTION_LONG,
      osm2ttl::config::constants::WRITE_GEOM_RELATION_STATISTICS_OPTION_HELP);

  auto outputOp = op.add<popl::Value<std::string>>(
      osm2ttl::config::constants::OUTPUT_OPTION_SHORT,
      osm2ttl::config::constants::OUTPUT_OPTION_LONG,
      osm2ttl::config::constants::OUTPUT_OPTION_HELP, "");
  auto outputFormatOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          osm2ttl::config::constants::OUTPUT_FORMAT_OPTION_SHORT,
          osm2ttl::config::constants::OUTPUT_FORMAT_OPTION_LONG,
          osm2ttl::config::constants::OUTPUT_FORMAT_OPTION_HELP, outputFormat);
  auto outputNoCompressOp = op.add<popl::Switch, popl::Attribute::advanced>(
      osm2ttl::config::constants::OUTPUT_NO_COMPRESS_OPTION_SHORT,
      osm2ttl::config::constants::OUTPUT_NO_COMPRESS_OPTION_LONG,
      osm2ttl::config::constants::OUTPUT_NO_COMPRESS_OPTION_HELP);
  auto cacheOp = op.add<popl::Value<std::string>>(
      osm2ttl::config::constants::CACHE_OPTION_SHORT,
      osm2ttl::config::constants::CACHE_OPTION_LONG,
      osm2ttl::config::constants::CACHE_OPTION_HELP, cache);

  try {
    op.parse(argc, argv);

    if (helpOp->count() > 0) {
      if (helpOp->count() == 1) {
        std::cerr << op << "\n";
      } else if (helpOp->count() == 2) {
        std::cerr << op.help(popl::Attribute::advanced) << "\n";
      } else {
        std::cerr << op.help(popl::Attribute::expert) << "\n";
      }
      exit(osm2ttl::config::ExitCode::SUCCESS);
    }

    if (!op.unknown_options().empty()) {
      std::cerr << "Unknown argument(s) specified:\n";
      for (const auto& o : op.unknown_options()) {
        std::cerr << o << "\n";
      }
      std::cerr << "\n" << op.help() << "\n";
      exit(osm2ttl::config::ExitCode::UNKNOWN_ARGUMENT);
    }

    // Skip passes
    noFacts = noFactsOp->is_set();
    noGeometricRelations = noGeometricRelationsOp->is_set();
    storeLocationsOnDisk = storeLocationsOnDiskOp->is_set();

    // Select types to dump
    noAreaFacts = noAreaFactsOp->is_set();
    noNodeFacts = noNodeFactsOp->is_set();
    noRelationFacts = noRelationFactsOp->is_set();
    noWayFacts = noWayFactsOp->is_set();

    noAreaGeometricRelations = noAreaGeometricRelationsOp->is_set();
    noNodeGeometricRelations = noNodeGeometricRelationsOp->is_set();
    noWayGeometricRelations = noWayGeometricRelationsOp->is_set();

    noAreaFacts |= noAreasOp->is_set();
    noAreaGeometricRelations |= noAreasOp->is_set();
    noNodeFacts |= noNodesOp->is_set();
    noNodeGeometricRelations |= noNodesOp->is_set();
    noRelationFacts |= noRelationsOp->is_set();
    noWayFacts |= noWaysOp->is_set();
    noWayGeometricRelations |= noWaysOp->is_set();

    // Select amount to dump
    addAreaEnvelope = addAreaEnvelopeOp->is_set();
    addAreaEnvelopeRatio = addAreaEnvelopeRatioOp->is_set();
    addNodeEnvelope = addNodeEnvelopeOp->is_set();
    addWayEnvelope = addWayEnvelopeOp->is_set();
    addWayMetadata = addWayMetaDataOp->is_set();
    addWayNodeOrder = addWayNodeOrderOp->is_set();
    adminRelationsOnly = adminRelationsOnlyOp->is_set();
    minimalAreaEnvelopeRatio = minimalAreaEnvelopeRatioOp->value();
    skipWikiLinks = skipWikiLinksOp->is_set();
    simplifyGeometries = simplifyGeometriesOp->value();
    simplifyWKT = simplifyWKTOp->value();
    wktDeviation = wktDeviationOp->value();
    wktPrecision = wktPrecisionOp->value();

    osm2ttlPrefix = osm2ttlPrefixOp->value();
    if (semicolonTagKeysOp->is_set()) {
      for (size_t i = 0; i < semicolonTagKeysOp->count(); ++i) {
        semicolonTagKeys.insert(semicolonTagKeysOp->value(i));
      }
    }

    // Dot
    writeDAGDotFiles = writeDotFilesOp->is_set();

    writeGeometricRelationStatistics = writeStatisticsOp->is_set();

    // Output
    output = outputOp->value();
    outputFormat = outputFormatOp->value();
    outputCompress = !outputNoCompressOp->is_set();
    if (output.empty()) {
      outputCompress = false;
      mergeOutput = util::OutputMergeMode::NONE;
    }
    statisticsPath = std::filesystem::path(output);
    statisticsPath += osm2ttl::config::constants::STATS_EXTENSION;
    if (outputCompress && !output.empty() &&
        output.extension() != osm2ttl::config::constants::BZIP2_EXTENSION) {
      output += osm2ttl::config::constants::BZIP2_EXTENSION;
      statisticsPath += osm2ttl::config::constants::BZIP2_EXTENSION;
    }

    // osmium location cache
    cache = std::filesystem::absolute(cacheOp->value()).string();

    // Check cache location
    if (!std::filesystem::exists(cache)) {
      std::cerr << "Cache location does not exist: " << cache << "\n"
                << op.help() << "\n";
      exit(osm2ttl::config::ExitCode::CACHE_NOT_EXISTS);
    }
    if (!std::filesystem::is_directory(cache)) {
      std::cerr << "Cache location not a directory: " << cache << "\n"
                << op.help() << "\n";
      exit(osm2ttl::config::ExitCode::CACHE_NOT_DIRECTORY);
    }

    // Handle input
    if (op.non_option_args().size() != 1) {
      std::cerr << "No input specified!\n" << op.help() << "\n";
      exit(osm2ttl::config::ExitCode::INPUT_MISSING);
    }
    input = op.non_option_args()[0];
    if (!std::filesystem::exists(input)) {
      std::cerr << "Input does not exist: " << input << "\n"
                << op.help() << "\n";
      exit(osm2ttl::config::ExitCode::INPUT_NOT_EXISTS);
    }
    if (std::filesystem::is_directory(input)) {
      std::cerr << "Input is a directory: " << input << "\n"
                << op.help() << "\n";
      exit(osm2ttl::config::ExitCode::INPUT_IS_DIRECTORY);
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
    exit(osm2ttl::config::ExitCode::FAILURE);
  }
}

// ____________________________________________________________________________
std::filesystem::path osm2ttl::config::Config::getTempPath(
    const std::string& p, const std::string& s) const {
  std::filesystem::path path{cache};
  path /= p + "-" + s;
  return std::filesystem::absolute(path);
}
