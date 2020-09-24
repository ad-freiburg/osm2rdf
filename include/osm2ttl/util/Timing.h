// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_UTIL_TIMING_H_
#define OSM2TTL_UTIL_TIMING_H_

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>

namespace osm2ttl {
namespace util {

class Timing {
 public:
  void start() { _start = std::chrono::steady_clock::now(); }
  void end() {
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> dur =
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
            end - _start);
    _min = std::min<std::chrono::duration<double, std::milli>>(_min, dur);
    _max = std::max<std::chrono::duration<double, std::milli>>(_max, dur);
    _count++;
    _mean += dur;
  }

  std::string toString() {
    std::ostringstream oss;
    oss << "min:   " << _min.count() << "\n";
    oss << "max:   " << _max.count() << "\n";
    oss << "mean:  " << (_mean.count() / _count) << "\n";
    oss << "count: " << _count;
    return oss.str();
  }

 private:
  size_t _count;
  std::chrono::_V2::steady_clock::time_point _start;
  std::chrono::duration<double, std::milli> _max;
  std::chrono::duration<double, std::milli> _min;
  std::chrono::duration<double, std::milli> _mean;
};

}  // namespace util
}  // namespace osm2ttl

#endif  // OSM2TTL_UTIL_TIMING_H_
