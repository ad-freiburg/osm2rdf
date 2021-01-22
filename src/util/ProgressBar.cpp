// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/ProgressBar.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <iostream>

// ____________________________________________________________________________
osm2ttl::util::ProgressBar::ProgressBar(std::size_t maxValue, bool show)
    : _maxValue(maxValue),
      _show(show),
      _countWidth(floor(log10(maxValue)) + 1),
      _width(80 - _countWidth * 2 - 4 - 5 -2),
      _percent(101),
      _last(std::time(nullptr)) {}

// ____________________________________________________________________________
void osm2ttl::util::ProgressBar::update(std::size_t count) {
  if (!_show) {
    return;
  }
  const std::size_t percent = 100 * (count) / _maxValue;
  const auto num = static_cast<std::size_t>(percent * (_width / 100.0));
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
