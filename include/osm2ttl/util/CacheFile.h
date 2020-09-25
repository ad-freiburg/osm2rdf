// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_CACHEFILE_H_
#define OSM2TTL_UTIL_CACHEFILE_H_

#include <filesystem>

namespace osm2ttl {
namespace util {

class CacheFile {
 public:
  explicit CacheFile(const std::filesystem::path& path);
  ~CacheFile();
  void reopen();
  void close();
  bool remove();
  [[nodiscard]] int fileDescriptor() const;

 protected:
  std::filesystem::path _path;
  int _fileDescriptor = -1;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_CACHEFILE_H_
