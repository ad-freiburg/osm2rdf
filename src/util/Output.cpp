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

#include <bzlib.h>

#include <cmath>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <thread>
#include <cassert>
#include <vector>

#if defined(_OPENMP)
#include "omp.h"
#endif
#include "osm2rdf/config/Config.h"
#include "osm2rdf/util/Output.h"

// ____________________________________________________________________________
osm2rdf::util::Output::Output(const osm2rdf::config::Config& config,
                              const std::string& prefix)
    : Output(config, prefix,
#if defined(_OPENMP)
             std::max(std::thread::hardware_concurrency(),
                      static_cast<unsigned int>(omp_get_max_threads()) + 1)
#else
             std::thread::hardware_concurrency() + 1
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
      _partCountDigits(std::floor(std::log10(_numOuts)) + 1),
      _outBuffers(_partCount + 2),
      _toStdOut(_config.output.empty()) {}

// ____________________________________________________________________________
osm2rdf::util::Output::~Output() { close(); }

// ____________________________________________________________________________
bool osm2rdf::util::Output::open() {
  assert(_partCount > 0);
  assert(_numOuts == _partCount + 2);

  _rawFiles.resize(_numOuts);
  _files.resize(_numOuts);
  _outBufPos.resize(_numOuts);

  for (size_t i = 0; i < _partCount + 2; i++) {
    if (i < _partCount) {
      _rawFiles[i] = fopen(partFilename(i).c_str(), "w");
    } else {
      _rawFiles[i] = fopen(partFilename(_partCount - i - 1).c_str(), "w");
    }
    int err = 0;
    _files[i] = BZ2_bzWriteOpen(&err, _rawFiles[i], 6, 0, 30);
    if (err != BZ_OK) {
      throw std::runtime_error("Could not open bzip file for writing.");
    }
    _outBuffers[i] = new unsigned char[BUFFER_S];
  }

  // Prepare final output file
  if (!_toStdOut && _config.mergeOutput != OutputMergeMode::NONE) {
    _outFile.open(_prefix, std::ofstream::out | std::ofstream::trunc);
    if (!_outFile.is_open()) {
      std::cerr << "Can't open final output file: " << _prefix << std::endl;
      return false;
    }
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

  if (_toStdOut) {
    for (size_t i = 0; i < _numOuts; ++i) {
      _outBuffers[i][_outBufPos[i]] = 0;
      fputs(reinterpret_cast<const char*>(_outBuffers[i]), stdout);
    }
  } else {
    for (size_t i = 0; i < _numOuts; ++i) {
      int err = 0;
      BZ2_bzWrite(&err, _files[i], _outBuffers[i], _outBufPos[i]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
        throw std::runtime_error("Could not write to file.");
      }
      BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
      fclose(_rawFiles[i]);
    }
  }

  _open = false;

  // Handle merging of files
  switch (_config.mergeOutput) {
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
  std::ifstream inFilePrefix{filename,
                             std::ifstream::in | std::ifstream::binary};
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
  std::ifstream inFileSuffix{filename,
                             std::ifstream::in | std::ifstream::binary};
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
void osm2rdf::util::Output::writeNewLine() {
#if defined(_OPENMP)
  writeNewLine(omp_get_thread_num());
#else
  writeNewLine(0);
#endif
}

// ____________________________________________________________________________
void osm2rdf::util::Output::writeNewLine(size_t part) { write('\n', part); }

// ____________________________________________________________________________
void osm2rdf::util::Output::write(std::string_view strv) {
#if defined(_OPENMP)
  write(strv, omp_get_thread_num());
#else
  write(strv, 0);
#endif
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(std::string_view strv, size_t t) {
  assert(part < _numOuts);
  if (_toStdOut) {
    if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
      _outBuffers[t][_outBufPos[t]] = 0;
      fputs(reinterpret_cast<const char*>(_outBuffers[t]), stdout);
      _outBufPos[t] = 0;
    }
  } else {
    if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
      int err = 0;
      BZ2_bzWrite(&err, _files[t], _outBuffers[t], _outBufPos[t]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[t], 0, 0, 0);
        throw std::runtime_error("Could not write to file.");
      }
      _outBufPos[t] = 0;
    }
  }

  memcpy(_outBuffers[t] + _outBufPos[t], strv.data(), strv.size());
  _outBufPos[t] += strv.size();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(const char c) {
#if defined(_OPENMP)
  write(c, omp_get_thread_num());
#else
  write(c, 0);
#endif
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(const char c, size_t t) {
  assert(part < _numOuts);
  if (_toStdOut) {
    if (_outBufPos[t] + 1 >= BUFFER_S) {
      _outBuffers[t][_outBufPos[t]] = 0;
      fputs(reinterpret_cast<const char*>(_outBuffers[t]), stdout);
      _outBufPos[t] = 0;
    }
  } else {
    if (_outBufPos[t] + 1 >= BUFFER_S) {
      int err = 0;
      BZ2_bzWrite(&err, _files[t], _outBuffers[t], _outBufPos[t]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[t], 0, 0, 0);
        throw std::runtime_error("Could not write to file.");
      }
      _outBufPos[t] = 0;
    }
  }

  *(_outBuffers[t] + _outBufPos[t]) = c;
  _outBufPos[t] += 1;
}

// ____________________________________________________________________________
void osm2rdf::util::Output::flush() {
  for (size_t i = 0; i < _numOuts; ++i) {
    flush(i);
  }
}

// ____________________________________________________________________________
void osm2rdf::util::Output::flush(size_t i) {
  if (_toStdOut) {
    _outBuffers[i][_outBufPos[i]] = 0;
    fputs(reinterpret_cast<const char*>(_outBuffers[i]), stdout);
  } else {
    int err = 0;
    BZ2_bzWrite(&err, _files[i], _outBuffers[i], _outBufPos[i]);
    if (err == BZ_IO_ERROR) {
      BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
      throw std::runtime_error("Could not write to file.");
    }
  }
}
