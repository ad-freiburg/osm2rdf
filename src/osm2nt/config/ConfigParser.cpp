// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/config/ConfigParser.h"

#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>

#include "osm2nt/config/Config.h"

// ____________________________________________________________________________
void osm2nt::config::ConfigParser::parse(Config* config,
                                         int argc, char** argv) {
  struct option opts[] = {
    {"output", required_argument, 0, 'o'},
    {"config", required_argument, 0, 'c'},
    {"tmp-cache", required_argument, 0, 't'},
    {"ignore-unnamed", no_argument, 0, 'u'},
    {0, 0, 0, 0}
  };

  char c;
  while ((c = getopt_long(argc, argv, "t:c:o:u", opts, 0)) != -1) {
    switch (c) {
    case 'o':
      config->output = std::string(optarg);
      break;
    case 't':
      config->cache = std::string(optarg);
      break;
    case 'u':
      config->ignoreUnnamed = true;
      break;
    case 'h':
      _usage(&std::cout, argv[0]);
      exit(0);
    case '?':
      std::cerr << "Unknown option " << argv[optind - 1] << std::endl;
      _usage(&std::cerr, argv[0]);
      exit(1);
    case ':':
      std::cerr << "Missing argument for " << argv[optind - 1] << std::endl;
      _usage(&std::cerr, argv[0]);
      exit(1);
    default:
      std::cerr << "Error while parsing arguments" << std::endl;
      _usage(&std::cerr, argv[0]);
      exit(1);
    }
  }

  if (optind > argc - 1) {
    _usage(&std::cerr, argv[0]);
    exit(1);
  }
  config->input = argv[optind];
}

// ____________________________________________________________________________
void osm2nt::config::ConfigParser::_usage(std::ostream* os,
                                          const char* binary) {
  *os << std::setfill(' ') << std::left << " " << VERSION_FULL << "\n\n"
      << "Usage: " << binary << " [options]" << " <OSM FILE>"
      << "\n\n"
      << "Options:\n"
      << "..." << std::endl;
}

