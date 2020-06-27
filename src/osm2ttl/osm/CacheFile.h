// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_CACHEFILE_H_
#define OSM2TTL_OSM_CACHEFILE_H_

#include <string>

namespace osm2ttl {
namespace osm {

class CacheFile {
 public:
  explicit CacheFile(const std::string& filename);
  bool open();
  bool close();
  void remove();
  int fileDescriptor() const;
 protected:
  std::string _filename;
  int _fileDescriptor = -1;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_CACHEFILE_H_
