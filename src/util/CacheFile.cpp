// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/CacheFile.h"

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>

// ____________________________________________________________________________
osm2ttl::util::CacheFile::CacheFile(const std::filesystem::path& path)
    : _path(std::filesystem::absolute(path)) {
  reopen();
}

// ____________________________________________________________________________
osm2ttl::util::CacheFile::~CacheFile() {
  close();
  remove();
}

// ____________________________________________________________________________
void osm2ttl::util::CacheFile::reopen() {
  const int RWRWRW = 0666;
  _fileDescriptor = ::open(_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, RWRWRW);
  if (_fileDescriptor == -1) {
    throw std::filesystem::filesystem_error(
        "Can't open CacheFile", std::filesystem::absolute(_path.c_str()),
        std::make_error_code(std::errc::permission_denied));
  }
}

// ____________________________________________________________________________
void osm2ttl::util::CacheFile::close() {
  if (_fileDescriptor >= 0) {
    ::close(_fileDescriptor);
    _fileDescriptor = -1;
  }
}

// ____________________________________________________________________________
bool osm2ttl::util::CacheFile::remove() {
  return std::filesystem::remove(_path);
}

// ____________________________________________________________________________
int osm2ttl::util::CacheFile::fileDescriptor() const { return _fileDescriptor; }
