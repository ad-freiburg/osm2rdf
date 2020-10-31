// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_OUTPUT_H
#define OSM2TTL_UTIL_OUTPUT_H

#include <fstream>

#include "boost/iostreams/filtering_stream.hpp"
#include "osm2ttl/config/Config.h"

namespace osm2ttl {
namespace util {

class Output {
 public:
  Output(const osm2ttl::config::Config& config, const std::string& prefix);
  ~Output();
  void open();
  void close();
  void merge(std::string_view prefix, std::string_view suffix);
  void write(std::string_view line);
  void write(std::string_view line, int part);

 protected:
  const osm2ttl::config::Config _config;
  const std::string _prefix;
  size_t _numFilesPerType;
  // Output
  boost::iostreams::filtering_ostream* _out;
  std::ofstream* _outFile;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_OUTPUT_H
