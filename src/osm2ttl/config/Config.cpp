// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/config/Config.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "popl.hpp"

#include "osm2ttl/ttl/OutputFormat.h"

// ____________________________________________________________________________
void osm2ttl::config::Config::load(const std::string& filename) {
}

// ____________________________________________________________________________
void osm2ttl::config::Config::save(const std::string& filename) {
}

// ____________________________________________________________________________
void osm2ttl::config::Config::fromArgs(int argc, char** argv) {
  popl::OptionParser op("Allowed options");

  auto helpOp = op.add<popl::Switch>("h", "help", "Lorem ipsum");
  auto configOp = op.add<popl::Value<std::string>>(
    "c", "config", "Config file");

  auto skipAreaPrepOp = op.add<popl::Switch, popl::Attribute::advanced>(
    "", "skip-area-prep", "Skip area sorting");
  auto useRamForLocationsOp = op.add<popl::Switch, popl::Attribute::advanced>(
    "", "use-ram-for-locations", "Store locations in RAM");
  auto numThreadsWriteOp = op.add<popl::Value<size_t>,
       popl::Attribute::advanced>("", "num-threads-write",
                                  "Number of threads to write output",
                                  numThreadsWrite);
  auto queueFactorWriteOp = op.add<popl::Value<size_t>,
       popl::Attribute::advanced>("", "queue-factor-write",
                                  "Factor for write queue",
                                  queueFactorWrite);
  auto numThreadsReadOp = op.add<popl::Value<size_t>,
       popl::Attribute::advanced>("", "num-threads-read",
                                  "Number of threads to read libosmium data",
                                  numThreadsRead);
  auto queueFactorReadOp = op.add<popl::Value<size_t>,
       popl::Attribute::advanced>("", "queue-factor-read",
                                  "Factor for read queue",
                                  queueFactorRead);
  auto numThreadsConvertGeomOp = op.add<popl::Value<size_t>,
       popl::Attribute::advanced>("", "num-threads-convert-geom",
                                  "Number of threads to convert geometries",
                                  numThreadsConvertGeom);
  auto queueFactorConvertGeomOp = op.add<popl::Value<size_t>,
       popl::Attribute::advanced>("", "queue-factor-convert-geom",
                                  "Factor for convert geometries queue",
                                  queueFactorConvertGeom);

  auto noNodeDumpOp = op.add<popl::Switch, popl::Attribute::advanced>("",
    "no-node-dump", "Skip nodes while dumping data");
  auto noRelationDumpOp = op.add<popl::Switch, popl::Attribute::advanced>("",
    "no-relation-dump", "Skip relations while dumping data");
  auto noWayDumpOp = op.add<popl::Switch, popl::Attribute::advanced>("",
    "no-way-dump", "Skip ways while dumping data");
  auto noAreaDumpOp = op.add<popl::Switch, popl::Attribute::advanced>("",
    "no-area-dump", "Skip areas while dumping data");

  auto addAreaSourcesOp = op.add<popl::Switch, popl::Attribute::advanced>(
    "", "add-area-sources", "Add area sources (ways and relations).");
  auto addEnvelopeOp = op.add<popl::Switch>(
    "", "add-envelope", "Add envelope to entries.");
  auto addMemberNodesOp = op.add<popl::Switch, popl::Attribute::advanced>(
    "", "add-member-nodes", "Add nodes triples for members of ways and" \
    "relations. This does not add information to the ways or relations.");
  auto skipWikiLinksOp = op.add<popl::Switch>(
    "w", "skip-wiki-links", "Skip addition of links to wikipedia/wikidata.");
  auto storeConfigOp = op.add<popl::Value<std::string>,
       popl::Attribute::advanced>(
    "", "store-config", "Path to store calculated config.");
  auto expandedDataOp = op.add<popl::Switch>("x", "expanded-data",
    "Add expanded data");
  auto metaDataOp = op.add<popl::Switch>("m", "meta-data",
    "Add meta-data");

  auto wktSimplifyOp = op.add<popl::Switch>(
    "s", "wkt-simplify", "Simplify WKT-Geometry");

  auto outputOp = op.add<popl::Value<std::string>>(
    "o", "output", "Output file", "");
  auto outputFormatOp = op.add<popl::Value<std::string>,
       popl::Attribute::advanced>(
    "", "output-format", "Output format, valid values: nt, ttl, qlever",
    "qlever");
  auto cacheOp = op.add<popl::Value<std::string>>(
    "t", "cache", "Path to cache directory", "/tmp/");
  auto gzipOp = op.add<popl::Switch>("z", "gzip", "Compress output");

  try {
    op.parse(argc, argv);

    if (helpOp->count() > 0) {
      if (helpOp->count() == 1) {
        std::cout << op << "\n";
      } else if (helpOp->count() == 2) {
        std::cout << op.help(popl::Attribute::advanced) << "\n";
      } else {
        std::cout << op.help(popl::Attribute::expert) << "\n";
      }
      exit(0);
    }

    // Handle config
    if (configOp->is_set()) {
      load(configOp->value());
    }

    // Skip passes
    skipAreaPrep = skipAreaPrepOp->is_set();
    useRamForLocations = useRamForLocationsOp->is_set();

    // Threads
    numThreadsConvertGeom = numThreadsConvertGeomOp->value();
    queueFactorConvertGeom = queueFactorConvertGeomOp->value();
    numThreadsRead = numThreadsReadOp->value();
    queueFactorRead = queueFactorReadOp->value();
    numThreadsWrite = numThreadsWriteOp->value();
    queueFactorWrite = queueFactorWriteOp->value();

    // Select types to dump
    noNodeDump = noNodeDumpOp->is_set();
    noRelationDump = noRelationDumpOp->is_set();
    noWayDump = noWayDumpOp->is_set();
    noAreaDump = noAreaDumpOp->is_set();

    // Select amount to dump
    addAreaSources = addAreaSourcesOp->is_set();
    addEnvelope = addEnvelopeOp->is_set();
    addMemberNodes = addMemberNodesOp->is_set();
    skipWikiLinks = skipWikiLinksOp->is_set();
    expandedData = expandedDataOp->is_set();
    wktSimplify = wktSimplifyOp->is_set();

    // Add tag.key() -> xsd overrides
    tagKeyType["admin_level"] = osm2ttl::ttl::IRI("xsd", "integer");

    // Output
    output = outputOp->value();
    if (outputFormatOp->is_set()) {
      if (outputFormatOp->value() == "ttl") {
        outputFormat = osm2ttl::ttl::OutputFormat::TTL;
      } else if (outputFormatOp->value() == "nt") {
        outputFormat = osm2ttl::ttl::OutputFormat::NT;
      } else if (outputFormatOp->value() == "qlever") {
        outputFormat = osm2ttl::ttl::OutputFormat::QLEVER;
      } else {
        std::cerr << "Unknown output format selected: "
          << outputFormatOp->value() << "\n"
          << op.help(popl::Attribute::advanced) << "\n";
        exit(1);
      }
    }
    gzip = gzipOp->is_set();

    // osmium location cache
    if (cacheOp->is_set() || cache.empty()) {
      cache = std::filesystem::absolute(cacheOp->value()).string()+"/";
    }

    // Check cache location
    if (!std::filesystem::is_directory(cache)) {
      std::cerr << "Cache location not a directory: "
        << cache << "\n"
        << op.help() << "\n";
      exit(1);
    }

    // Handle input
    if (op.non_option_args().size() != 1) {
      std::cerr << op << "\n";
      exit(1);
    }
    input = op.non_option_args()[0];
    if (!std::filesystem::exists(input)) {
      std::cerr << "Input does not exist: "
        << input << "\n"
        << op.help() << "\n";
      exit(1);
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
      std::string option_name(e.option()->name(popl::OptionName::short_name,
                                               true));
      if (option_name.empty()) {
        option_name = e.option()->name(popl::OptionName::long_name, true);
      }
      std::cerr << "option: " << option_name << "\n";
    } else {
      std::cerr << "option: " << e.option()->name(e.what_name()) << "\n";
      std::cerr << "value:  " << e.value() << "\n";
    }
    exit(1);
  }
}

// ____________________________________________________________________________
std::filesystem::path osm2ttl::config::Config::getTempPath(
  const std::string& p, const std::string& s) const {
  std::filesystem::path path{cache};
  path /= p+"-"+s;
  return std::filesystem::absolute(path);
}
