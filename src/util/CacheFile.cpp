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
