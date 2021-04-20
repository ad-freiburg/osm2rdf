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

#ifndef OSM2TTL_UTIL_CACHEFILE_H_
#define OSM2TTL_UTIL_CACHEFILE_H_

#include <filesystem>

namespace osm2ttl::util {

class CacheFile {
 public:
  // Creates CacheFile at given path.
  explicit CacheFile(const std::filesystem::path& path);
  // Closes and removes files.
  ~CacheFile();
  // Opens file.
  void reopen();
  // Closes file if open.
  void close();
  // Removes file.
  bool remove();
  // Returns file descriptor for use in libosmium.
  [[nodiscard]] int fileDescriptor() const;

 protected:
  std::filesystem::path _path;
  int _fileDescriptor = -1;
};

}  // namespace osm2ttl::util

#endif  // OSM2TTL_UTIL_CACHEFILE_H_
