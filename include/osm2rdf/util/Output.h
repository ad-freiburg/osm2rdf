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

#include <bzlib.h>
#include <fstream>
#include <vector>

#include "osm2rdf/config/Config.h"

static const size_t BUFFER_S = 1024 * 1024 * 50;

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

  // Write the given string view into the specified part.
  void write(std::string_view strv, size_t part); // Flush all part.

  // Write the given char view into the specified part.
  void write(const char c, size_t part); // Flush all part.

  // Write a newline into the specified part.
  void writeNewLine(size_t part);

  void flush();
  // Flush the given part.
  void flush(size_t part);
  // Filename for given part. Special handling for -1 (prefix) and -2 (suffix).
  std::string partFilename(int part);

 protected:
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
  // Final output file
  std::ofstream _outFile;

  std::stringstream* _outBufs;

  std::vector<unsigned char*> _outBuffers;

  std::vector<FILE*> _rawFiles;
  std::vector<BZFILE*> _files;
  std::vector<size_t> _outBufPos;

  // true if output goes to stdout
  bool _toStdOut;
};

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_OUTPUT_H
