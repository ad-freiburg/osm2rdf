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
  Output(const osm2ttl::config::Config& config, const std::string& prefix,
         size_t partCount);
  ~Output();
  // open all output streams.
  bool open();
  // close all output streams.
  void close();
  void close(std::string_view prefix, std::string_view suffix);
  // write the given line into the correct part for the current (openmp) thread.
  void write(std::string_view line);
  // write the given line into the specified part.
  void write(std::string_view line, size_t part);

 protected:
  // merge closes and merges all parts, prepend given prefix and append given suffix.
  void merge(std::string_view prefix, std::string_view suffix);
  void concatinate(std::string_view prefix, std::string_view suffix);
  void none(std::string_view prefix, std::string_view suffix);
  const osm2ttl::config::Config _config;
  const std::string _prefix;
  size_t _numOuts;
  bool _open = false;
  // Output
  boost::iostreams::filtering_ostream* _out;
  std::ofstream* _outFile;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_OUTPUT_H
