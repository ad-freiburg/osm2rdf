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

#ifndef OSM2TTL_UTIL_PROGRESSBAR_H_
#define OSM2TTL_UTIL_PROGRESSBAR_H_

static const int k100Percent = 100;
static const int kTerminalWidth = 80;
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
