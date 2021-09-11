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

#ifndef OSM2RDF_UTIL_TIMING_H_
#define OSM2RDF_UTIL_TIMING_H_

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>

namespace osm2rdf::util {

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
  size_t _count = 0;
  std::chrono::_V2::steady_clock::time_point _start;
  std::chrono::duration<double, std::milli> _max =
      std::chrono::duration<double, std::milli>::min();
  std::chrono::duration<double, std::milli> _min =
      std::chrono::duration<double, std::milli>::max();
  std::chrono::duration<double, std::milli> _mean =
      std::chrono::duration<double, std::milli>::zero();
};

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_TIMING_H_
