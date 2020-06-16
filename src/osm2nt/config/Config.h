// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_CONFIG_CONFIG_H_
#define OSM2NT_CONFIG_CONFIG_H_

#include <string>
#include <unordered_map>

#include "osm2nt/nt/OutputFormat.h"

namespace osm2nt {
namespace config {

struct Config {
  void load(const std::string& filename);
  void save(const std::string& filename);
  void fromArgs(int argc, char** argv);

  bool simplifyWKT = false;
  bool addWikiLinks = false;
  bool ignoreUnnamed = false;
  std::string output;
  osm2nt::nt::OutputFormat outputFormat = osm2nt::nt::OutputFormat::TTL;
  std::string input;
  std::string cache;
  std::unordered_map<std::string, std::string> prefixes;
};
}  // namespace config
}  // namespace osm2nt

#endif  // OSM2NT_CONFIG_CONFIG_H_

