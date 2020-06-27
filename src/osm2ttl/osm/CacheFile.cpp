// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/CacheFile.h"

#include <fcntl.h>
#include <unistd.h>
#include <string>

// ____________________________________________________________________________
osm2ttl::osm::CacheFile::CacheFile(const std::string& filename) :
  _filename(filename) {}

// ____________________________________________________________________________
bool osm2ttl::osm::CacheFile::open() {
  const int RWRWRW = 0666;
  _fileDescriptor = ::open(_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC,
                           RWRWRW);
  return _fileDescriptor != -1;
}

// ____________________________________________________________________________
bool osm2ttl::osm::CacheFile::close() {
  if (_fileDescriptor >= 0) {
    ::close(_fileDescriptor);
    _fileDescriptor = -1;
    return true;
  }
  return false;
}

// ____________________________________________________________________________
void osm2ttl::osm::CacheFile::remove() {
  std::remove(_filename.c_str());
}

// ____________________________________________________________________________
int osm2ttl::osm::CacheFile::fileDescriptor() const {
  return _fileDescriptor;
}
