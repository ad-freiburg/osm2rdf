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

#include "osm2rdf/util/Output.h"

#include <cmath>
#include <cstdio>
#include <iostream>

#include "boost/iostreams/filter/bzip2.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "omp.h"
#include "osm2rdf/config/Config.h"

// ____________________________________________________________________________
osm2rdf::util::Output::Output(const osm2rdf::config::Config& config,
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
osm2rdf::util::Output::Output(const osm2rdf::config::Config& config,
                              const std::string& prefix, size_t partCount)
    : _config(config),
      _prefix(prefix),
      _partCount(partCount),
      _numOuts(_partCount + 2),
      _partCountDigits(std::floor(std::log10(_numOuts)) + 1) {}

// ____________________________________________________________________________
osm2rdf::util::Output::~Output() { close(); }

// ____________________________________________________________________________
bool osm2rdf::util::Output::open() {
  assert(_partCount > 0);
  assert(_numOuts == _partCount + 2);
  _outs = new boost::iostreams::filtering_ostream[_numOuts];
  _outFiles = new std::ofstream[_numOuts];

  // Prepare final output file
  if (!_config.output.empty() && _config.mergeOutput != OutputMergeMode::NONE) {
    _outFile.open(_prefix, std::ofstream::out | std::ofstream::trunc);
    if (!_outFile.is_open()) {
      std::cerr << "Can't open final output file: " << _prefix << std::endl;
      return false;
    }
  }

  // One part for each thread
  for (size_t i = 0; i < _partCount; ++i) {
    if (_config.outputCompress) {
      _outs[i].push(boost::iostreams::bzip2_compressor{});
    }
    if (_config.output.empty()) {
      _outs[i].push(std::cout);
    } else {
      _outFiles[i].open(partFilename(i),
                        std::ofstream::out | std::ofstream::trunc);
      if (!_outFiles[i].is_open()) {
        std::cerr << "Can't open part file: " << partFilename(i) << std::endl;
        return false;
      }
      _outs[i].push(_outFiles[i]);
    }
  }

  // One for prefix
  if (_config.outputCompress) {
    _outs[_partCount].push(boost::iostreams::bzip2_compressor{});
  }
  if (_config.output.empty()) {
    _outs[_partCount].push(std::cout);
  } else {
    _outFiles[_partCount].open(partFilename(-1));
    if (!_outFiles[_partCount].is_open()) {
      std::cerr << "Can't open prefix file: " << partFilename(-1) << std::endl;
      return false;
    }
    _outs[_partCount].push(_outFiles[_partCount]);
  }

  // One for suffix
  if (_config.outputCompress) {
    _outs[_partCount + 1].push(boost::iostreams::bzip2_compressor{});
  }
  if (_config.output.empty()) {
    _outs[_partCount + 1].push(std::cout);
  } else {
    _outFiles[_partCount + 1].open(partFilename(-2));
    if (!_outFiles[_partCount + 1].is_open()) {
      std::cerr << "Can't open suffix file: " << partFilename(-2) << std::endl;
      return false;
    }
    _outs[_partCount + 1].push(_outFiles[_partCount + 1]);
  }

  _open = true;
  return true;
}

// ____________________________________________________________________________
void osm2rdf::util::Output::close() { close("", ""); }

// ____________________________________________________________________________
void osm2rdf::util::Output::close(std::string_view prefix,
                                  std::string_view suffix) {
  if (!_open) {
    return;
  }

  // Write prefix and suffix to file
  write(prefix, _partCount);
  write(suffix, _partCount + 1);

  for (size_t i = 0; i < _numOuts; ++i) {
    _outs[i].pop();
    if (_outFiles[i].is_open()) {
      _outFiles[i].close();
    }
  }
  delete[] _outFiles;
  delete[] _outs;
  _open = false;

  // Handle merging of files
  switch (_config.mergeOutput) {
    case osm2rdf::util::OutputMergeMode::MERGE:
      merge();
      break;
    case osm2rdf::util::OutputMergeMode::CONCATENATE:
      concatenate();
      break;
    case osm2rdf::util::OutputMergeMode::NONE:
    default:
      break;
  }

  // close final file
  _outFile.flush();
  _outFile.close();
}

// ____________________________________________________________________________
std::string osm2rdf::util::Output::partFilename(int part) {
  assert(part >= -2);
  assert(part < static_cast<int>(_partCount));
  std::ostringstream oss;
  oss << _prefix << ".part_" << std::setfill('0')
      << std::setw(_partCountDigits);
  if (part == -2) {
    part = static_cast<int>(_partCount);
  }
  oss << (part + 1);
  return oss.str();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::concatenate() {
  // Reopen outfile as binary
  _outFile.close();
  _outFile.open(_prefix, std::ofstream::out | std::ofstream::binary);
  if (!_outFile.is_open()) {
    std::cerr << "Can't reopen file: " << _prefix << " keeping files!"
              << std::endl;
    return;
  }

  // Prefix
  std::string filename = partFilename(-1);
  std::ifstream inFilePrefix{filename, std::ifstream::in | std::ifstream::binary};
  if (!inFilePrefix.is_open() || !inFilePrefix.good()) {
    std::cerr << "Error opening file: " << filename << std::endl;
  }
  _outFile << inFilePrefix.rdbuf();
  inFilePrefix.close();
  if (!_config.outputKeepFiles) {
    std::filesystem::remove(filename);
  }

  // Content
  for (size_t i = 0; i < _partCount; ++i) {
    filename = partFilename(i);
    std::ifstream inFile{filename, std::ifstream::in | std::ifstream::binary};
    if (!inFile.is_open() || !inFile.good()) {
      std::cerr << "Error opening file: " << filename << std::endl;
    }
    _outFile << inFile.rdbuf();
    inFile.close();
    if (!_config.outputKeepFiles) {
      std::filesystem::remove(filename);
    }
  }

  // Suffix
  filename = partFilename(-2);
  std::ifstream inFileSuffix{filename, std::ifstream::in | std::ifstream::binary};
  if (!inFileSuffix.is_open() || !inFileSuffix.good()) {
    std::cerr << "Error opening file: " << filename << std::endl;
  }
  _outFile << inFileSuffix.rdbuf();
  inFileSuffix.close();
  if (!_config.outputKeepFiles) {
    std::filesystem::remove(filename);
  }
}

// ____________________________________________________________________________
void osm2rdf::util::Output::merge() {
  boost::iostreams::filtering_ostream out;
  if (_config.outputCompress) {
    out.push(boost::iostreams::bzip2_compressor{});
  }
  out.push(_outFile);

  // Prefix
  boost::iostreams::filtering_istream in;
  std::string filename = partFilename(-1);
  std::ifstream inFile{filename};
  if (_config.outputCompress) {
    in.push(boost::iostreams::bzip2_decompressor{});
  }
  if (!inFile.is_open() || !inFile.good()) {
    std::cerr << "Error opening file: " << filename << std::endl;
  }
  in.push(inFile);
  out << in.rdbuf();
  in.pop();
  inFile.close();
  if (!_config.outputKeepFiles) {
    std::filesystem::remove(filename);
  }

  // Content
  for (size_t i = 0; i < _partCount; ++i) {
    filename = partFilename(i);
    inFile.open(filename);
    if (!inFile.is_open() || !inFile.good()) {
      std::cerr << "Error opening file: " << filename << std::endl;
    }
    in.push(inFile);
    out << in.rdbuf();
    in.pop();
    inFile.close();
    if (!_config.outputKeepFiles) {
      std::filesystem::remove(filename);
    }
  }
  // Suffix
  filename = partFilename(-2);
  inFile.open(filename);
  if (!inFile.is_open() || !inFile.good()) {
    std::cerr << "Error opening file: " << filename << std::endl;
  }
  in.push(inFile);
  out << in.rdbuf();
  in.pop();
  inFile.close();
  if (!_config.outputKeepFiles) {
    std::filesystem::remove(filename);
  }

  out.flush();
  out.pop();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(std::string_view line) {
#if defined(_OPENMP)
  write(line, omp_get_thread_num());
#else
  write(line, 0);
#endif
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(std::string_view line, size_t part) {
  assert(part < _numOuts);
  _outs[part] << line;
}

// ____________________________________________________________________________
void osm2rdf::util::Output::flush() {
  for (size_t i = 0; i < _numOuts; ++i) {
    flush(i);
  }
}

// ____________________________________________________________________________
void osm2rdf::util::Output::flush(size_t part) { _outs[part].flush(); }