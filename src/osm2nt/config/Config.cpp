// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/config/Config.h"

#include <string>
#include <iostream>

#include "popl.hpp"

#include "osm2nt/nt/OutputFormat.h"

// ____________________________________________________________________________
void osm2nt::config::Config::load(const std::string& filename) {
}

// ____________________________________________________________________________
void osm2nt::config::Config::save(const std::string& filename) {
}

// ____________________________________________________________________________
void osm2nt::config::Config::fromArgs(int argc, char** argv) {
  popl::OptionParser op("Allowed options");

  auto helpOp = op.add<popl::Switch>("h", "help", "Lorem ipsum");
  auto configOp = op.add<popl::Value<std::string>>(
    "c", "config", "Config file");
  auto outputOp = op.add<popl::Value<std::string>>(
    "o", "output", "Output file", "");
  auto outputFormatOp = op.add<popl::Value<std::string>,
       popl::Attribute::advanced>(
    "", "output-format", "Output format, valid values: nt, ttl", "ttl");
  auto cacheOp = op.add<popl::Value<std::string>>(
    "t", "cache", "Path to cache file", "/tmp/osm2nt-cache");
  auto ignoreUnnamedOp = op.add<popl::Switch>(
    "u", "ignore-unnamed", "Only add named entities to the result.");
  auto addWikiLinksOp = op.add<popl::Switch>(
    "w", "add-wiki-links", "Add links to wikipedia and wikidata.");
  auto simplifyWKTOp = op.add<popl::Switch>(
    "s", "simplify-wkt", "Simplify WKT-Geometry");
  auto storeConfigOp = op.add<popl::Value<std::string>,
       popl::Attribute::advanced>(
    "", "store-config", "Path to store calculated config.");

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
    // Set values
    output = outputOp->value();
    if (outputFormatOp->is_set()) {
      if (outputFormatOp->value() == "ttl") {
        outputFormat = osm2nt::nt::OutputFormat::TTL;
      } else if (outputFormatOp->value() == "nt") {
        outputFormat = osm2nt::nt::OutputFormat::NT;
      } else {
        std::cerr << "Unknown output format selected: "
          << outputFormatOp->value() << "\n"
          << op.help(popl::Attribute::advanced) << "\n";
        exit(1);
      }
    }
    if (cacheOp->is_set() || cache.empty()) {
      cache = cacheOp->value();
    }
    if (ignoreUnnamedOp->is_set()) {
      ignoreUnnamed = true;
    }
    if (addWikiLinksOp->is_set()) {
      addWikiLinks = true;
    }
    if (simplifyWKTOp->is_set()) {
      simplifyWKT = true;
    }

    // Handle input
    if (op.non_option_args().size() != 1) {
      std::cerr << op << "\n";
      exit(1);
    }
    input = op.non_option_args()[0];
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
