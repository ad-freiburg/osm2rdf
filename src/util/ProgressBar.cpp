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

#include "osm2ttl/util/ProgressBar.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>

// ____________________________________________________________________________
osm2ttl::util::ProgressBar::ProgressBar(std::size_t maxValue, bool show)
    : _maxValue(maxValue),
      _countWidth(std::floor(std::log10(maxValue)) + 1),
      _percent(k100Percent + 1),
      _last(std::time(nullptr)),
      _show(show) {
  // Handle special case of 0 elements
  if (maxValue == 0) {
    _countWidth = 1;
  }
  _width = kTerminalWidth - _countWidth * 2 - 4 - 5 - 2;
}

// ____________________________________________________________________________
void osm2ttl::util::ProgressBar::update(std::size_t count) {
  if (!_show) {
    return;
  }
  std::size_t percent = k100Percent;
  if (_maxValue > 0) {
    percent = k100Percent * (count) / _maxValue;
  }
  const auto num = static_cast<std::size_t>(percent * (_width / 100.0));
  assert(num <= _width);

  // Only update if percent changed or timediff > 1 second
  if (_percent == percent && std::difftime(std::time(nullptr), _last) < 1) {
    return;
  }

  _percent = percent;
  _oldValue = count;
  std::cerr << '[';
  for (size_t i = 0; i < num; ++i) {
    std::cerr << '=';
  }
  if (num < _width) {
    std::cerr << '>';
  }
  for (size_t i = num + 1; i < _width; ++i) {
    std::cerr << ' ';
  }
  std::cerr << ']';

  // %
  std::cerr << ' ' << std::setw(3) << std::right << percent << "%";

  // [x/y]
  std::cerr << " [" << std::setw(_countWidth) << std::right << count << "/"
            << _maxValue << "]\r";

  // Update last update time
  _last = std::time(nullptr);
}

// ____________________________________________________________________________
void osm2ttl::util::ProgressBar::done() {
  if (!_show) {
    return;
  }
  update(_maxValue);
  std::cerr << std::endl;
}

// ____________________________________________________________________________
std::size_t osm2ttl::util::ProgressBar::countWidth() const {
  return _countWidth;
}