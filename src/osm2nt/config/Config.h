// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_CONFIG_CONFIG_H_
#define OSM2NT_CONFIG_CONFIG_H_

#include <string>

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
  std::string input;
  std::string cache = "/tmp/osm2nt-cache";
};
}  // namespace config
}  // namespace osm2nt

#endif  // OSM2NT_CONFIG_CONFIG_H_

