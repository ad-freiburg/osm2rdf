// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONFIG_H_
#define OSM2TTL_CONFIG_CONFIG_H_

#include <string>
#include <unordered_map>

#include "osm2ttl/ttl/OutputFormat.h"

namespace osm2ttl {
namespace config {

struct Config {
  size_t simplifyWKT = 0;
  bool addUnnamed = false;
  bool expandedData = false;
  bool skipWikiLinks = false;
  bool skipFirstPass = true;
  bool skipSecondPass = false;
  bool skipAreaPrep = false;
  std::string output;
  osm2ttl::ttl::OutputFormat outputFormat = osm2ttl::ttl::OutputFormat::QLEVER;
  std::string input;
  std::string cache;

  std::unordered_map<std::string, osm2ttl::ttl::IRI> tagKeyType;

  void load(const std::string& filename);
  void save(const std::string& filename);
  void fromArgs(int argc, char** argv);

  static Config& getInstance() {
    static Config instance;
    return instance;
  }

 private:
  Config() {}
};
}  // namespace config
}  // namespace osm2ttl

#endif  // OSM2TTL_CONFIG_CONFIG_H_

