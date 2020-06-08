// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_CONFIG_CONFIGPARSER_H_
#define OSM2NT_CONFIG_CONFIGPARSER_H_

#include <getopt.h>
#include <ostream>

#include "osm2nt/_config.h"
#include "osm2nt/config/Config.h"

namespace osm2nt {
namespace config {

class ConfigParser {
 public:
  static void parse(Config* config, int argc, char** argv);
 protected:
  static void _usage(std::ostream* os, const char* bin);
};
}  // namespace config
}  // namespace osm2nt

#endif  // OSM2NT_CONFIG_CONFIGPARSER_H_
