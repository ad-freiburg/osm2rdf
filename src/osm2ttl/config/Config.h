// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONFIG_H_
#define OSM2TTL_CONFIG_CONFIG_H_

#include <string>

#include "osm2ttl/ttl/OutputFormat.h"

namespace osm2ttl {
namespace config {

struct Config {
  bool simplifyWKT = false;
  bool addWikiLinks = false;
  bool ignoreUnnamed = false;
  std::string output;
  osm2ttl::ttl::OutputFormat outputFormat = osm2ttl::ttl::OutputFormat::TTL;
  std::string input;
  std::string cache;

  void load(const std::string& filename);
  void save(const std::string& filename);
  void fromArgs(int argc, char** argv);
};
}  // namespace config
}  // namespace osm2ttl

#endif  // OSM2TTL_CONFIG_CONFIG_H_

