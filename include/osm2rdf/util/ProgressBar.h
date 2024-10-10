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

#ifndef OSM2RDF_UTIL_PROGRESSBAR_H_
#define OSM2RDF_UTIL_PROGRESSBAR_H_

static const int k100Percent = 100;
static const int kTerminalWidth = 80;
#include <cstdio>
#include <ctime>

namespace osm2rdf::util {

class ProgressBar {
 public:
  // Initializes a ProgressBar with given maxValue. If show equals false nothing
  // will be printed to std::cerr when update is called.
  ProgressBar(std::size_t maxValue, bool show);
  ProgressBar() = default;
  // Updates the progress bar.
  void update(std::size_t count);
  // Marks progress bar as done (calling update with _maxValue).
  void done();

  // Returns number of digits required to print _maxValue.
  std::size_t countWidth() const;

 protected:
  // Maximal value for this progress bar.
  std::size_t _maxValue;
  // Current absolute value.
  std::size_t _oldValue;
  // Number of digits required for _maxValue.
  std::size_t _countWidth;
  // Width of whole progress bar.
  std::size_t _width;
  // Current percent value.
  std::size_t _percent;
  // Time of last update.
  std::time_t _last;
  // Print to std::cerr or not.
  bool _show = false;
};

}  // namespace osm2rdf::util

#endif  // OSM2RDF_UTIL_PROGRESSBAR_H_
