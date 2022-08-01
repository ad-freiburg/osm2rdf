// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2RDF_UTIL_OUTPUT_H
#define OSM2RDF_UTIL_OUTPUT_H

#include <fstream>

#include "boost/iostreams/filtering_stream.hpp"
#include "osm2rdf/config/Config.h"

namespace osm2rdf::util {

class Output {
 public:
  Output(const osm2rdf::config::Config& config, const std::string& prefix);
  Output(const osm2rdf::config::Config& config, const std::string& prefix,
         size_t partCount);
  ~Output();
  // Create and open all output streams.
  bool open();
  // Close all output streams.
  void close();
  void close(std::string_view prefix, std::string_view suffix);
  // Write the given line into the correct part for the current (openmp) thread.
  void write(std::string_view line);
  // Write the given line into the specified part.
  void write(std::string_view line, size_t part);
  // Flush all part.
  void flush();
  // Flush the given part.
  void flush(size_t part);
  // Filename for given part. Special handling for -1 (prefix) and -2 (suffix).
  std::string partFilename(int part);

 protected:
  // Closes and merges all parts, prepend given prefix and append given suffix.
  void merge();
  // Closes and concatenates all parts without decompressing and recompressing
  // streams.
  void concatenate();
  // Config instance.
  const osm2rdf::config::Config _config;
  // Prefix for all filenames.
  const std::string _prefix;
  // Number of parts.
  std::size_t _partCount;
  // Number of temporary output streams.
  std::size_t _numOuts;
  // Number of digits required for _partCount.
  std::size_t _partCountDigits;
  bool _open = false;
  // Output streams
  boost::iostreams::filtering_ostream* _outs;
  std::ofstream* _outFiles;
  // Final output file
  std::ofstream _outFile;
};

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_OUTPUT_H
