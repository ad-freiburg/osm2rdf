// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/util/Output.h"

#include <cmath>
#include <cstdio>
#include <iostream>

#include "boost/iostreams/filter/bzip2.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "omp.h"
#include "osm2ttl/config/Config.h"

// ____________________________________________________________________________
osm2ttl::util::Output::Output(const osm2ttl::config::Config& config,
                              const std::string& prefix)
    : Output(config, prefix,
#if defined(_OPENMP)
             omp_get_max_threads()
#else
             1
#endif
      ) {
}

// ____________________________________________________________________________
osm2ttl::util::Output::Output(const osm2ttl::config::Config& config,
                              const std::string& prefix, size_t partCount)
    : _config(config),
      _prefix(prefix),
      _numOuts(partCount),
      _numOutsDigits(std::floor(std::log10(_numOuts)) + 1) {}

// ____________________________________________________________________________
osm2ttl::util::Output::~Output() { close(); }

// ____________________________________________________________________________
bool osm2ttl::util::Output::open() {
  assert(_numOuts > 0);
  _out = new boost::iostreams::filtering_ostream[_numOuts];
  _outFile = new std::ofstream[_numOuts];

  for (size_t i = 0; i < _numOuts; ++i) {
    if (_config.outputCompress) {
      _out[i].push(boost::iostreams::bzip2_compressor{});
    }
    if (_config.output.empty()) {
      _out[i].push(std::cout);
    } else {
      _outFile[i].open(partFilename(i));
      if (!_outFile[i].is_open()) {
        return false;
      }
      _out[i].push(_outFile[i]);
    }
  }
  _open = true;
  return true;
}

// ____________________________________________________________________________
void osm2ttl::util::Output::close() { close("", ""); }

// ____________________________________________________________________________
void osm2ttl::util::Output::close(std::string_view prefix,
                                  std::string_view suffix) {
  if (!_open) {
    return;
  }

  for (size_t i = 0; i < _numOuts; ++i) {
    _out[i].pop();
    if (_outFile[i].is_open()) {
      _outFile[i].close();
    }
  }
  delete[] _outFile;
  delete[] _out;
  _open = false;

  // Handle merging of files
  switch (_config.mergeOutput) {
    case osm2ttl::util::OutputMergeMode::MERGE:
      merge(prefix, suffix);
      return;
    case osm2ttl::util::OutputMergeMode::CONCATENATE:
      concatenate(prefix, suffix);
      return;
    case osm2ttl::util::OutputMergeMode::NONE:
    default:
      none(prefix, suffix);
      return;
  }
}

// ____________________________________________________________________________
std::string osm2ttl::util::Output::partFilename(int part) {
  assert(part >= -2);
  assert(part < static_cast<int>(_numOuts));
  std::ostringstream oss;
  oss << _prefix << ".part_" << std::setfill('0') << std::setw(_numOutsDigits);
  if (part == -2) {
    part = static_cast<int>(_numOuts);
  }
  oss << (part + 1);
  return oss.str();
}

// ____________________________________________________________________________
void osm2ttl::util::Output::merge(std::string_view prefix,
                                  std::string_view suffix) {
  // Concatenated output files
  boost::iostreams::filtering_ostream out;
  std::ofstream outFile{_prefix};
  if (_config.outputCompress) {
    out.push(boost::iostreams::bzip2_compressor{});
  }
  out.push(outFile);
  out << prefix;

  for (size_t i = 0; i < _numOuts; ++i) {
    boost::iostreams::filtering_istream in;
    std::string filename = partFilename(i);
    std::ifstream inFile{filename};
    if (_config.outputCompress) {
      in.push(boost::iostreams::bzip2_decompressor{});
    }
    in.push(inFile);
    out << in.rdbuf();
    in.pop();
    inFile.close();
    std::remove(filename.c_str());
  }

  out << suffix;
  out.pop();
  outFile.close();
}

// ____________________________________________________________________________
void osm2ttl::util::Output::concatenate(std::string_view prefix,
                                        std::string_view suffix) {
  // Concatenated output files
  std::ofstream outFile{_prefix, std::ios_base::binary};
  outFile << prefix;

  for (size_t i = 0; i < _numOuts; ++i) {
    std::string filename = partFilename(i);
    std::ifstream inFile{filename, std::ios_base::binary};
    outFile << inFile.rdbuf();
    std::remove(filename.c_str());
  }

  outFile << suffix;
  outFile.close();
}

// ____________________________________________________________________________
void osm2ttl::util::Output::none(std::string_view prefix,
                                 std::string_view suffix) {
  // Concatenated output files
  {
    std::ofstream outFile{partFilename(-1)};
    outFile << prefix;
    outFile.close();
  }
  {
    std::ofstream outFile{partFilename(-2)};
    outFile << suffix;
    outFile.close();
  }
}

// ____________________________________________________________________________
void osm2ttl::util::Output::write(std::string_view line) {
#if defined(_OPENMP)
  write(line, omp_get_thread_num());
#else
  write(line, 0);
#endif
}

// ____________________________________________________________________________
void osm2ttl::util::Output::write(std::string_view line, size_t part) {
  assert(part < _numOuts);
  _out[part] << line;
}

// ____________________________________________________________________________
void osm2ttl::util::Output::flush() {
  for (size_t i = 0; i < _numOuts; ++i) {
    flush(i);
  }
}

// ____________________________________________________________________________
void osm2ttl::util::Output::flush(size_t part) { _out[part].flush(); }