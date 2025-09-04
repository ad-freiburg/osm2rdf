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

#include <bzlib.h>
#include <zlib.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include "osm2rdf/config/Config.h"
#include "osm2rdf/util/Time.h"

using osm2rdf::config::BZ2;
using osm2rdf::config::GZ;
using osm2rdf::config::NONE;

// ____________________________________________________________________________
osm2rdf::util::Output::Output(const osm2rdf::config::Config& config,
                              const std::string& prefix)
    : Output(config, prefix, config.numThreads + 1) {}

// ____________________________________________________________________________
osm2rdf::util::Output::Output(const osm2rdf::config::Config& config,
                              const std::string& prefix, size_t partCount)
    : _config(config),
      _prefix(prefix),
      _partCount(partCount),
      _partCountDigits(std::floor(std::log10(partCount)) + 1),
      _outBuffers(_partCount),
      _lines(_partCount),
      _toStdOut(_config.output.empty()) {}

// ____________________________________________________________________________
osm2rdf::util::Output::~Output() { close(); }

// ____________________________________________________________________________
bool osm2rdf::util::Output::open() {
  assert(_partCount > 0);

  std::cerr << zlibVersion() << std::endl;

  _rawFiles.resize(_partCount);
  _gzFiles.resize(_partCount);
  _files.resize(_partCount);
  _outBufPos.resize(_partCount);

  for (size_t i = 0; i < _partCount; i++) {
    if (_config.outputCompress == BZ2 || _config.outputCompress == NONE) {
      _rawFiles[i] = fopen(partFilename(i).c_str(), "w");

      if (_rawFiles[i] == NULL) {
        std::stringstream ss;
        ss << "Could not open file '" << partFilename(i) << "' for writing:\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
    }

    if (_config.outputCompress == BZ2) {
      int err = 0;
      _files[i] = BZ2_bzWriteOpen(&err, _rawFiles[i], 3, 0, 30);
      if (err != BZ_OK) {
        std::stringstream ss;
        ss << "Could not open bzip2 file '" << partFilename(i)
           << "' for writing:\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
    }

    if (_config.outputCompress == GZ) {
      _gzFiles[i] = gzopen(partFilename(i).c_str(), "w");
      if (_gzFiles[i] == Z_NULL) {
        std::stringstream ss;
        ss << "Could not open gz file '" << partFilename(i)
           << "' for writing:\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
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
    for (size_t i = 0; i < _partCount; ++i) {
      _lines[i] = 0;
      _outBuffers[i][_outBufPos[i]] = '\0';
      std::cout << reinterpret_cast<const char*>(_outBuffers[i]);
    }
  } else if (_config.outputCompress == BZ2) {
#pragma omp parallel for
    for (size_t i = 0; i < _partCount; ++i) {
      int err = 0;
      BZ2_bzWrite(&err, _files[i], _outBuffers[i], _outBufPos[i]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
        std::stringstream ss;
        ss << "Could not write to bzip2 file '" << partFilename(i) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
      fclose(_rawFiles[i]);
    }
  } else if (_config.outputCompress == GZ) {
#pragma omp parallel for
    for (size_t i = 0; i < _partCount; ++i) {
      int r = gzwrite(_gzFiles[i], _outBuffers[i], _outBufPos[i]);
      if (r != (int)_outBufPos[i]) {
        gzclose(_gzFiles[i]);
        std::stringstream ss;
        ss << "Could not write to gz file '" << partFilename(i) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      gzclose(_gzFiles[i]);
    }
  } else {
#pragma omp parallel for
    for (size_t i = 0; i < _partCount; ++i) {
      size_t r =
          fwrite(_outBuffers[i], sizeof(char), _outBufPos[i], _rawFiles[i]);
      if (r != _outBufPos[i]) {
        std::stringstream ss;
        ss << "Could not write to file '" << partFilename(i) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      fclose(_rawFiles[i]);
    }
  }

  for (size_t i = 0; i < _partCount; ++i) {
    delete[] _outBuffers[i];
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
    if (inFile.peek() != std::ifstream::traits_type::eof()) {
      _outFile << inFile.rdbuf();
    }
    inFile.close();
    if (!_config.outputKeepFiles) {
      std::filesystem::remove(filename);
    }
  }

  _outFile.flush();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::writeNewLine(size_t part) {
  write('\n', part);
  _lines[part]++;
  if (_toStdOut) {
    if (_lines[part] > 50) flush(part);
  }
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(std::string_view strv, size_t t) {
  assert(t < _partCount);
  if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
    if (_config.outputCompress == BZ2) {
      int err = 0;
      BZ2_bzWrite(&err, _files[t], _outBuffers[t], _outBufPos[t]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[t], 0, 0, 0);
        std::stringstream ss;
        ss << "Could not write to bzip2 file '" << partFilename(t) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      _outBufPos[t] = 0;
    } else if (_config.outputCompress == GZ) {
      int r = gzwrite(_gzFiles[t], _outBuffers[t], _outBufPos[t]);
      if (r != (int)_outBufPos[t]) {
        gzclose(_gzFiles[t]);
        std::stringstream ss;
        ss << "Could not write to gz file '" << partFilename(t) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      _outBufPos[t] = 0;
    } else if (_config.outputCompress == NONE) {
      size_t r =
          fwrite(_outBuffers[t], sizeof(char), _outBufPos[t], _rawFiles[t]);
      if (r != _outBufPos[t]) {
        std::stringstream ss;
        ss << "Could not write to file '" << partFilename(t) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      _outBufPos[t] = 0;
    }
  }

  if (_outBufPos[t] + strv.size() + 1 >= BUFFER_S) {
    throw std::runtime_error("Write buffer too small to write " +
                             std::to_string(strv.size()) + " bytes");
  }

  memcpy(_outBuffers[t] + _outBufPos[t], strv.data(), strv.size());
  _outBufPos[t] += strv.size();
}

// ____________________________________________________________________________
void osm2rdf::util::Output::write(const char c, size_t t) {
  assert(t < _partCount);
  if (_outBufPos[t] + 2 >= BUFFER_S) {
    if (_config.outputCompress == BZ2) {
      int err = 0;
      BZ2_bzWrite(&err, _files[t], _outBuffers[t], _outBufPos[t]);
      if (err == BZ_IO_ERROR) {
        BZ2_bzWriteClose(&err, _files[t], 0, 0, 0);
        std::stringstream ss;
        ss << "Could not write to bzip2 file '" << partFilename(t) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      _outBufPos[t] = 0;
    } else if (_config.outputCompress == GZ) {
      int r = gzwrite(_gzFiles[t], _outBuffers[t], _outBufPos[t]);
      if (r != (int)_outBufPos[t]) {
        gzclose(_gzFiles[t]);
        std::stringstream ss;
        ss << "Could not write to gz file '" << partFilename(t) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      _outBufPos[t] = 0;
    } else if (_config.outputCompress == NONE) {
      size_t r =
          fwrite(_outBuffers[t], sizeof(char), _outBufPos[t], _rawFiles[t]);
      if (r != _outBufPos[t]) {
        std::stringstream ss;
        ss << "Could not write to file '" << partFilename(t) << "':\n";
        ss << strerror(errno) << std::endl;
        throw std::runtime_error(ss.str());
      }
      _outBufPos[t] = 0;
    }
  }

  if (_outBufPos[t] + 2 >= BUFFER_S) {
    throw std::runtime_error("Write buffer too small to write 1 byte");
  }

  *(_outBuffers[t] + _outBufPos[t]) = c;
  _outBufPos[t] += 1;
}

// ____________________________________________________________________________
void osm2rdf::util::Output::flush() {
  for (size_t i = 0; i < _partCount; ++i) {
    flush(i);
  }
}

// ____________________________________________________________________________
void osm2rdf::util::Output::flush(size_t i) {
  if (_toStdOut) {
    _lines[i] = 0;
    _outBuffers[i][_outBufPos[i]] = '\0';
    std::cout << reinterpret_cast<const char*>(_outBuffers[i]);
  } else if (_config.outputCompress == BZ2) {
    int err = 0;
    BZ2_bzWrite(&err, _files[i], _outBuffers[i], _outBufPos[i]);
    if (err == BZ_IO_ERROR) {
      BZ2_bzWriteClose(&err, _files[i], 0, 0, 0);
      std::stringstream ss;
      ss << "Could not write to bzip2 file '" << partFilename(i) << "':\n";
      ss << strerror(errno) << std::endl;
      throw std::runtime_error(ss.str());
    }
  } else if (_config.outputCompress == GZ) {
    int r = gzwrite(_gzFiles[i], _outBuffers[i], _outBufPos[i]);
    if (r != (int)_outBufPos[i]) {
      gzclose(_gzFiles[i]);
      std::stringstream ss;
      ss << "Could not write to gz file '" << partFilename(i) << "':\n";
      ss << strerror(errno) << std::endl;
      throw std::runtime_error(ss.str());
    }
  } else {
    size_t r =
        fwrite(_outBuffers[i], sizeof(char), _outBufPos[i], _rawFiles[i]);
    if (r != _outBufPos[i]) {
      std::stringstream ss;
      ss << "Could not write to file '" << partFilename(i) << "':\n";
      ss << strerror(errno) << std::endl;
      throw std::runtime_error(ss.str());
    }
  }
  _outBufPos[i] = 0;
}
