// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/Output.h"

#include <fstream>
#include <cstdio>

#include "boost/iostreams/filter/bzip2.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "omp.h"
#include "osm2ttl/config/Config.h"

// ____________________________________________________________________________
osm2ttl::util::Output::Output(const osm2ttl::config::Config& config,
                              const std::string& prefix)
    : _config(config), _prefix(prefix) {
#if defined(_OPENMP)
  _numOuts = omp_get_max_threads();
#else
  _numOuts = 1;
#endif
}

// ____________________________________________________________________________
osm2ttl::util::Output::~Output() {
  close();
}

// ____________________________________________________________________________
void osm2ttl::util::Output::open() {
  _out = new boost::iostreams::filtering_ostream[_numOuts];
  _outFile = new std::ofstream[_numOuts];

  for (size_t i = 0; i < _numOuts; ++i) {
    if (_config.outputCompress) {
      _out[i].push(boost::iostreams::bzip2_compressor{});
    }
    _outFile[i].open(_prefix + ".part_" + std::to_string(i));
    _out[i].push(_outFile[i]);
  }
  _open = true;
}

// ____________________________________________________________________________
void osm2ttl::util::Output::close() {
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
}

void osm2ttl::util::Output::merge(std::string_view prefix, std::string_view suffix) {
  // Close and flush all data before merging.
  close();
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
    std::ifstream inFile{_prefix + ".part_" + std::to_string(i)};
    if (_config.outputCompress) {
      in.push(boost::iostreams::bzip2_decompressor{});
    }
    in.push(inFile);
    out << in.rdbuf();
    in.pop();
    inFile.close();
    std::remove(std::string(_prefix + ".part_" + std::to_string(i)).c_str());
  }

  out << suffix;
  out.pop();
  outFile.close();
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