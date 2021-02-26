// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/config/Config.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "omp.h"
#include "osm2ttl/config/Constants.h"
#include "osm2ttl/config/ExitCode.h"
#include "osm2ttl/ttl/Format.h"
#include "popl.hpp"

// ____________________________________________________________________________
std::string osm2ttl::config::Config::getInfo(std::string_view prefix) const {
  std::ostringstream oss;
  oss << prefix << osm2ttl::config::constants::HEADER;
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_IO;
  oss << "\n" << prefix << "Input:         " << input;
  oss << "\n" << prefix << "Output:        " << output;
  oss << "\n" << prefix << "Output format: " << outputFormat;
  oss << "\n" << prefix << "Cache:         " << cache;
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_FACTS;
  oss << "\n" << prefix << "Prefix for own IRIs: " << osm2ttlPrefix;
  if (noFacts) {
    oss << "\n" << prefix << osm2ttl::config::constants::NO_FACTS;
  } else {
    if (noAreas) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_AREA_FACTS;
    } else {
      if (addAreaEnvelope) {
        oss << "\n" << prefix << osm2ttl::config::constants::ADD_AREA_ENVELOPE;
      }
    }
    if (noNodes) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_NODE_FACTS;
    }
    if (noRelations) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_RELATION_FACTS;
    }
    if (noWays) {
      oss << "\n" << prefix << osm2ttl::config::constants::NO_WAY_FACTS;
    } else {
      if (addWayEnvelope) {
        oss << "\n" << prefix << osm2ttl::config::constants::ADD_WAY_ENVELOPE;
      }
      if (addWayMetadata) {
        oss << "\n" << prefix << osm2ttl::config::constants::ADD_WAY_METADATA;
      }
      if (addWayNodeOrder) {
        oss << "\n" << prefix << osm2ttl::config::constants::ADD_WAY_NODE_ORDER;
      }
    }
  }
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_CONTAINS;
  if (noGeometricRelations) {
    oss << "\n" << prefix << osm2ttl::config::constants::NO_GEOM_RELATIONS;
  } else {
    if (noAreas) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_AREA_GEOM_RELATIONS;
    }
    if (noNodes) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_NODE_GEOM_RELATIONS;
    }
    if (noWays) {
      oss << "\n"
          << prefix << osm2ttl::config::constants::NO_WAY_GEOM_RELATIONS;
    }
    if (addInverseRelationDirection) {
      oss << "\n"
          << prefix
          << osm2ttl::config::constants::ADD_INVERSE_RELATION_DIRECTION;
    }
  }
  oss << "\n" << prefix << osm2ttl::config::constants::SECTION_MISCELLANEOUS;
  if (storeLocationsOnDisk) {
    oss << "\n" << prefix << "Storing locations osmium locations on disk!";
  }
  if (writeStatistics) {
    oss << "\n"
        << prefix << "Storing statistics about geometry calculations - SLOW!";
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

  auto helpOp = op.add<popl::Switch>("h", "help", "Lorem ipsum");

  auto noFactsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-facts", "Do not dump facts");
  auto noGeometricRelationsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-geometric-relations", "Do not calculate geometric relations");
  auto storeLocationsOnDiskOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "store-locations-on-disk", "Store locations in RAM");

  auto noAreasOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-areas", "Ignore areas");
  auto noNodesOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-nodes", "Ignore nodes");
  auto noRelationsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-relations", "Ignore relations");
  auto noWaysOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-ways", "Ignore ways");

  auto addAreaEnvelopeOp =
      op.add<popl::Switch>("", "add-area-envelope", "Add envelope to areas.");
  auto addWayEnvelopeOp =
      op.add<popl::Switch>("", "add-way-envelope", "Add envelope to ways.");
  auto addWayMetaDataOp = op.add<popl::Switch>(
      "", "add-way-meta-data", "Add information about the way structure.");
  auto addWayNodeOrderOp =
      op.add<popl::Switch>("", "add-way-node-order",
                           "Add information about the node members in ways.");

  auto addInverseRelationDirectionOp =
      op.add<popl::Switch, popl::Attribute::advanced>(
          "", "add-inverse-relation-direction",
          "Adds relations in the opposite direction");

  auto adminRelationsOnlyOp =
      op.add<popl::Switch>("", "admin-relations-only",
                           "Only dump nodes and relations with admin-level");
  auto skipWikiLinksOp = op.add<popl::Switch>(
      "w", "skip-wiki-links", "Skip addition of links to wikipedia/wikidata.");
  auto storeConfigOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          "", "store-config", "Path to store calculated config.");

  auto osm2ttlPrefixOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          "", "oms2ttl-prefix", "Prefix for own IRIs", osm2ttlPrefix);

  auto wktSimplifyOp = op.add<popl::Value<uint16_t>>(
      "s", "wkt-simplify",
      "Simplify WKT-Geometries over this number of nodes, 0 to disable",
      wktSimplify);

  auto writeDotFilesOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "write-dot-files", "Writes .dot files for DAGs");

  auto writeStatisticsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "write-statistics", "Writes statistic files");

  auto outputOp =
      op.add<popl::Value<std::string>>("o", "output", "Output file", "");
  auto outputFormatOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          "", "output-format", "Output format, valid values: nt, ttl, qlever",
          "qlever");
  auto outputNoCompressOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "output-no-compress", "Do not compress output");
  auto cacheOp = op.add<popl::Value<std::string>>(
      "t", "cache", "Path to cache directory", cache);

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

    // Skip passes
    noFacts = noFactsOp->is_set();
    noGeometricRelations = noGeometricRelationsOp->is_set();
    storeLocationsOnDisk = storeLocationsOnDiskOp->is_set();

    // Select types to dump
    noNodes = noNodesOp->is_set();
    noRelations = noRelationsOp->is_set();
    noWays = noWaysOp->is_set();
    noAreas = noAreasOp->is_set();

    // Select amount to dump
    addAreaEnvelope = addAreaEnvelopeOp->is_set();
    addInverseRelationDirection = addInverseRelationDirectionOp->is_set();
    addWayEnvelope = addWayEnvelopeOp->is_set();
    addWayMetadata = addWayMetaDataOp->is_set();
    addWayNodeOrder = addWayNodeOrderOp->is_set();
    adminRelationsOnly = adminRelationsOnlyOp->is_set();
    skipWikiLinks = skipWikiLinksOp->is_set();
    wktSimplify = wktSimplifyOp->value();

    osm2ttlPrefix = osm2ttlPrefixOp->value();

    // Dot
    writeDotFiles = writeDotFilesOp->is_set();

    writeStatistics = writeStatisticsOp->is_set();

    // Output
    output = outputOp->value();
    outputFormat = outputFormatOp->value();
    outputCompress = !outputNoCompressOp->is_set();
    if (output.empty()) {
      outputCompress = false;
      mergeOutput = util::OutputMergeMode::NONE;
    }
    statisticsPath = std::filesystem::path(output);
    statisticsPath += ".stats";
    if (outputCompress && !output.empty() && output.extension() != ".bz2") {
      output += ".bz2";
      statisticsPath += ".bz2";
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
