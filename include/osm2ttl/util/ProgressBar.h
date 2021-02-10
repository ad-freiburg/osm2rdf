// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_PROGRESSBAR_H_
#define OSM2TTL_UTIL_PROGRESSBAR_H_

#include <cstdio>
#include <ctime>

namespace osm2ttl::util {

class ProgressBar {
 public:
  ProgressBar(std::size_t maxValue, bool show);
  void update(std::size_t count);
  void done();

  std::size_t countWidth() const;

 protected:
  std::size_t _maxValue;
  std::size_t _oldValue;
  std::size_t _countWidth;
  std::size_t _width;
  std::size_t _percent;
  std::time_t _last;
  bool _show;
};

}  // namespace osm2ttl::util

#endif  // OSM2TTL_UTIL_PROGRESSBAR_H_
