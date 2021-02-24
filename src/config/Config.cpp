// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/config/Config.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "omp.h"
#include "osm2ttl/config/ExitCode.h"
#include "osm2ttl/ttl/Format.h"
#include "popl.hpp"

// ____________________________________________________________________________
void osm2ttl::config::Config::load(const std::string& filename) {}

// ____________________________________________________________________________
void osm2ttl::config::Config::save(const std::string& filename) {}

// ____________________________________________________________________________
std::string osm2ttl::config::Config::getInfo(std::string_view prefix) const {
  std::ostringstream oss;
  oss << prefix << "Config";
  oss << "\n" << prefix << "--- I/O ---";
  oss << "\n" << prefix << "Input:         " << input;
  oss << "\n" << prefix << "Output:        " << output;
  oss << "\n" << prefix << "Output format: " << outputFormat;
  oss << "\n" << prefix << "Cache:         " << cache;
  oss << "\n" << prefix << "--- Dump ---";
  oss << "\n" << prefix << "Prefix for own IRIs: " << osm2ttlPrefix;
  if (noDump) {
    oss << "\n" << prefix << "Not dumping facts";
  } else {
    if (noAreaDump) {
      oss << "\n" << prefix << "Ignoring areas in dump";
    }
    if (noNodeDump) {
      oss << "\n" << prefix << "Ignoring nodes in dump";
    }
    if (noRelationDump) {
      oss << "\n" << prefix << "Ignoring relations in dump";
    }
    if (noWayDump) {
      oss << "\n" << prefix << "Ignoring ways in dump";
    }
  }
  oss << "\n" << prefix << "--- Contains ---";
  if (noContains) {
    oss << "\n" << prefix << "Not dumping any geometric relations";
  } else {
    if (noAreaDump) {
      oss << "\n" << prefix << "Ignoring areas in geometric relations";
    }
    if (noNodeDump) {
      oss << "\n" << prefix << "Ignoring nodes in geometric relations";
    }
    if (noWayDump) {
      oss << "\n" << prefix << "Ignoring ways in geometric relations";
    }
    if (addInverseRelationDirection) {
      oss << "\n" << prefix << "Adding ogc:contained_by and ogc:intersected_by";
    }
  }
  oss << "\n" << prefix << "--- Misc ---";
  if (storeLocationsOnDisk) {
    oss << "\n" << prefix << "Storing locations osmium locations on disk!";
  }
  if (writeStatistics) {
    oss << "\n"
        << prefix << "Storing statistics about geometry calculations - SLOW!";
  }
  oss << "\n" << prefix << "--- OpenMP ---";
#if defined(_OPENMP)
  oss << "\n" << prefix << "Max Threads: " << omp_get_max_threads();
#else
  oss << "\n" << "Not available";
#endif
  return oss.str();
}

// ____________________________________________________________________________
void osm2ttl::config::Config::fromArgs(int argc, char** argv) {
  popl::OptionParser op("Allowed options");

  auto helpOp = op.add<popl::Switch>("h", "help", "Lorem ipsum");
  auto configOp =
      op.add<popl::Value<std::string>>("c", "config", "Config file");

  auto noDumpOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-dump", "Do not dump normal data");
  auto noContainsOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-contains", "Do not calculate contains relation");
  auto storeLocationsOnDiskOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "store-locations-on-disk", "Store locations in RAM");

  auto noNodeDumpOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-node-dump", "Skip nodes while dumping data");
  auto noRelationDumpOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-relation-dump", "Skip relations while dumping data");
  auto noWayDumpOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-way-dump", "Skip ways while dumping data");
  auto noAreaDumpOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "no-area-dump", "Skip areas while dumping data");

  auto addAreaSourcesOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "add-area-sources", "Add area sources (ways and relations).");
  auto addEnvelopeOp =
      op.add<popl::Switch>("", "add-envelope", "Add envelope to entries.");
  auto addMemberNodesOp = op.add<popl::Switch, popl::Attribute::advanced>(
      "", "add-member-nodes",
      "Add nodes triples for members of ways and"
      "relations. This does not add information to the ways or relations.");
  auto adminRelationsOnlyOp =
      op.add<popl::Switch>("", "admin-relations-only",
                           "Only dump nodes and relations with admin-level");
  auto skipWikiLinksOp = op.add<popl::Switch>(
      "w", "skip-wiki-links", "Skip addition of links to wikipedia/wikidata.");
  auto storeConfigOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          "", "store-config", "Path to store calculated config.");
  auto expandedDataOp =
      op.add<popl::Switch>("x", "expanded-data", "Add expanded data");

  auto osm2ttlPrefixOp =
      op.add<popl::Value<std::string>, popl::Attribute::advanced>(
          "", "oms2ttl-prefix", "Prefix for own IRIs", osm2ttlPrefix);

  auto wktSimplifyOp = op.add<popl::Value<uint16_t>>(
      "s", "wkt-simplify",
      "Simplify WKT-Geometries over this number of nodes, 0 to disable",
      wktSimplify);
  auto addInverseRelationDirectionOp =
      op.add<popl::Switch, popl::Attribute::advanced>(
          "", "add-inverse-relation-direction",
          "Adds relations in the opposite direction");

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

    // Handle config
    if (configOp->is_set()) {
      load(configOp->value());
    }

    // Skip passes
    noDump = noDumpOp->is_set();
    noContains = noContainsOp->is_set();
    storeLocationsOnDisk = storeLocationsOnDiskOp->is_set();

    // Select types to dump
    noNodeDump = noNodeDumpOp->is_set();
    noRelationDump = noRelationDumpOp->is_set();
    noWayDump = noWayDumpOp->is_set();
    noAreaDump = noAreaDumpOp->is_set();

    // Select amount to dump
    addAreaSources = addAreaSourcesOp->is_set();
    addEnvelope = addEnvelopeOp->is_set();
    addMemberNodes = addMemberNodesOp->is_set();
    adminRelationsOnly = adminRelationsOnlyOp->is_set();
    skipWikiLinks = skipWikiLinksOp->is_set();
    expandedData = expandedDataOp->is_set();
    wktSimplify = wktSimplifyOp->value();
    addInverseRelationDirection = addInverseRelationDirectionOp->is_set();

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
      std::cerr << "No input specified!\n"
                << op.help() << "\n";
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
