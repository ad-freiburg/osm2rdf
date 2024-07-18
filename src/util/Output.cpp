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

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <thread>
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
      _numOuts(_partCount),
      _partCountDigits(std::floor(std::log10(_numOuts)) + 1),
      _outBuffers(_partCount),
      _toStdOut(_config.output.empty()) {}

// ____________________________________________________________________________
osm2rdf::util::Output::~Output() { close(); }

// ____________________________________________________________________________
bool osm2rdf::util::Output::open() {
  assert(_partCount > 0);
  assert(_numOuts == _partCount);

  _rawFiles.resize(_numOuts);
  _files.resize(_numOuts);
  _outBufPos.resize(_numOuts);

  for (size_t i = 0; i < _partCount; i++) {
    _rawFiles[i] = fopen(partFilename(i).c_str(), "w");

    if (_config.outputCompress) {
      int err = 0;
      _files[i] = BZ2_bzWriteOpen(&err, _rawFiles[i], 6, 0, 30);
      if (err != BZ_OK) {
        throw std::runtime_error("Could not open bzip file for writing.");
      }
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
void osm2rdf::util::Output::close() {
  if (!_open) {
    return;
  }

  if (_toStdOut) {
    for (size_t i = 0; i < _numOuts; ++i) {
      _outBuffers[i][_outBufPos[i]] = 0;
      fputs(reinterpret_cast<const char*>(_outBuffers[i]), stdout);
    }
  } else if (_config.outputCompress) {
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
  } else {
    for (size_t i = 0; i < _numOuts; ++i) {
      size_t r =
          fwrite(_outBuffers[i], sizeof(char), _outBufPos[i], _rawFiles[i]);
      if (r != _outBufPos[i]) {
        throw std::runtime_error("Could not write to file.");
      }
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
  std::ostringstream oss;
  oss << _prefix << ".part_" << std::setfill('0')
      << std::setw(_partCountDigits);
  oss << (part);
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

  // Content
  for (size_t i = 0; i < _partCount; ++i) {
    std::string filename = partFilename(i);
    std::ifstream inFile{filename, std::ifstream::binary};
    if (!inFile.is_open() || !inFile.good()) {
      std::cerr << "Error opening file: " << filename << std::endl;
    }
    _outFile << inFile.rdbuf();
    inFile.close();
    if (!_config.outputKeepFiles) {
      std::filesystem::remove(filename);
    }
  }

  _outFile.flush();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::writeNewLine(size_t part) { write('\n', part); }

// ____________________________________________________________________________
void osm2rdf::util::Output::write(std::string_view strv, size_t t) {
  assert(t < _numOuts);
  if (_toStdOut) {
    if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
      _outBuffers[t][_outBufPos[t]] = 0;
      fputs(reinterpret_cast<const char*>(_outBuffers[t]), stdout);
      _outBufPos[t] = 0;
    }
  } else if (_config.outputCompress) {
    if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
      int err = 0;
      BZ2_bzWrite(&err, _files[t], _outBuffers[t], _outBufPos[t]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[t], 0, 0, 0);
        throw std::runtime_error("Could not write to file.");
      }
      _outBufPos[t] = 0;
    }
  } else {
    if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
      size_t r =
          fwrite(_outBuffers[t], sizeof(char), _outBufPos[t], _rawFiles[t]);
      if (r != _outBufPos[t]) {
        throw std::runtime_error("Could not write to file.");
      }
      _outBufPos[t] = 0;
    }
  }

  memcpy(_outBuffers[t] + _outBufPos[t], strv.data(), strv.size());
  _outBufPos[t] += strv.size();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(const char c, size_t t) {
  assert(t < _numOuts);
  if (_toStdOut) {
    if (_outBufPos[t] + 2 >= BUFFER_S) {
      _outBuffers[t][_outBufPos[t]] = 0;
      fputs(reinterpret_cast<const char*>(_outBuffers[t]), stdout);
      _outBufPos[t] = 0;
    }
  } else if (_config.outputCompress) {
    if (_outBufPos[t] + 2 >= BUFFER_S) {
      int err = 0;
      BZ2_bzWrite(&err, _files[t], _outBuffers[t], _outBufPos[t]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[t], 0, 0, 0);
        throw std::runtime_error("Could not write to file.");
      }
      _outBufPos[t] = 0;
    }
  } else {
    if (_outBufPos[t] + 2 >= BUFFER_S) {
      size_t r =
          fwrite(_outBuffers[t], sizeof(char), _outBufPos[t], _rawFiles[t]);
      if (r != _outBufPos[t]) {
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
  } else if (_config.outputCompress) {
    int err = 0;
    BZ2_bzWrite(&err, _files[i], _outBuffers[i], _outBufPos[i]);
    if (err == BZ_IO_ERROR) {
      BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
      throw std::runtime_error("Could not write to file.");
    }
  } else {
    size_t r =
        fwrite(_outBuffers[i], sizeof(char), _outBufPos[i], _rawFiles[i]);
    if (r != _outBufPos[i]) {
      throw std::runtime_error("Could not write to file.");
    }
  }
  _outBufPos[i] = 0;
}
