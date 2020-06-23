// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/SimplifyingWKTFactory.h"

#include <cmath>
#include <algorithm>
#include <utility>

#include "osmium/geom/coordinates.hpp"

#include "osm2ttl/config/Config.h"

// ____________________________________________________________________________
osm2ttl::osm::SimplifyingWKTFactoryImpl::SimplifyingWKTFactoryImpl(
  int /*unused*/, int precision) : _precision(precision) {
}

// ____________________________________________________________________________
double osm2ttl::osm::SimplifyingWKTFactoryImpl::getDistance(
  const osmium::geom::Coordinates& x, const osmium::geom::Coordinates& y) {
  const double a = x.x - y.x;
  const double b = x.y - y.y;
  return std::sqrt(a*a + b*b);
}

// ____________________________________________________________________________
double osm2ttl::osm::SimplifyingWKTFactoryImpl::getAngle(
  const osmium::geom::Coordinates& a, const osmium::geom::Coordinates& b,
  const osmium::geom::Coordinates& c) {
  return atan2(b.y - a.y, b.x - a.x) - atan2(c.y - a.y, c.x - a.x);
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::merge(const size_t index1,
  const size_t index2) {
  switch (_mergeMode) {
  case MergeMode::DELETE_FIRST:
    _coordinates.erase(_coordinates.begin() + index1);
    break;
  case MergeMode::DELETE_SECOND:
    _coordinates.erase(_coordinates.begin() + index2);
    break;
  case MergeMode::MERGE:
    const double x = (_coordinates[index1].x + _coordinates[index2].x) / 2;
    const double y = (_coordinates[index1].y + _coordinates[index2].y) / 2;
    _coordinates.erase(_coordinates.begin() + index2);
    _coordinates.erase(_coordinates.begin() + index1);
    _coordinates.emplace(_coordinates.begin() + index1, x, y);
    break;
  }
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::simplifyByAngle(const bool closed,
  const double angleDiff) {
  const double halfCircle = 180.0;
  // Close -> handle indizes n-1 0 1
  if (closed) {
    while (true) {
      if (_coordinates.size() < 3) {
        break;
      }
      const size_t pos0 = 0;
      const size_t pos1 = 1;
      const size_t pos2 = _coordinates.size() - 1;
      const double angle = std::abs(halfCircle - getAngle(_coordinates[pos0],
                                                          _coordinates[pos1],
                                                          _coordinates[pos2]));
      if (angle >= angleDiff) {
        break;
      }
      _coordinates.erase(_coordinates.begin() + pos0);
    }
  }
  // handle indizes 0 1 2, ... n-3 n-2 n-1
  for (size_t pos = 1;
       _coordinates.size() > 2 && pos < (_coordinates.size() - 2); ++pos) {
    while (true) {
      if (_coordinates.size() < 3) {
        break;
      }
      const size_t pos0 = pos;
      const size_t pos1 = pos - 1;
      const size_t pos2 = pos + 1;
      const double angle = std::abs(halfCircle - getAngle(_coordinates[pos0],
                                                          _coordinates[pos1],
                                                          _coordinates[pos2]));
      if (angle >= angleDiff) {
        break;
      }
      _coordinates.erase(_coordinates.begin() + pos0);
    }
  }
  // Close -> handle indizes n-2 n-1 0
  if (closed) {
    while (true) {
      if (_coordinates.size() < 3) {
        break;
      }
      const size_t pos0 = _coordinates.size() - 1;
      const size_t pos1 = _coordinates.size() - 2;
      const size_t pos2 = 0;
      const double angle = std::abs(halfCircle - getAngle(_coordinates[pos0],
                                                          _coordinates[pos1],
                                                          _coordinates[pos2]));
      if (angle >= angleDiff) {
        break;
      }
      _coordinates.erase(_coordinates.begin() + pos0);
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::simplifyByDistance(
  const bool closed, const double distanceDiff) {
  if (closed) {
    while (true) {
      if (_coordinates.size() < 2) {
        break;
      }
      const size_t pos0 = 0;
      const size_t pos1 = _coordinates.size() - 1;
      if (getDistance(_coordinates[pos0], _coordinates[pos1]) > distanceDiff) {
        break;
      }
      merge(pos0, pos1);
    }
  }
  for (size_t pos = 1; pos < (_coordinates.size() - 1); ++pos) {
    while (true) {
      // Abort if not enough coordinates remaining or we moved outside the list.
      if (_coordinates.size() < 2 || pos > (_coordinates.size() - 1)) {
        break;
      }
      const size_t pos0 = pos - 1;
      const size_t pos1 = pos;
      if (getDistance(_coordinates[pos0], _coordinates[pos1]) > distanceDiff) {
        break;
      }
      merge(pos0, pos1);
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::simplify(const bool closed) {
  const size_t maxCoords = osm2ttl::config::Config::getInstance().simplifyWKT;
  double minX = _coordinates[0].x;
  double minY = _coordinates[0].y;
  double maxX = _coordinates[0].x;
  double maxY = _coordinates[0].y;
  for (const osmium::geom::Coordinates& c : _coordinates) {
    minX = std::min(minX, c.x);
    minY = std::min(minY, c.y);
    maxX = std::max(maxX, c.x);
    maxY = std::max(maxY, c.y);
  }

  const double bboxSmallest = std::min(std::abs(maxX-minX),
                                       std::abs(maxY-minY));
  const double bboxScaleFactor = 100000.0;
  double distanceDiff = bboxSmallest / bboxScaleFactor;
  const double halfCircle = 180.0;
  double angleDiff = 2;
  while (_coordinates.size() > maxCoords) {
    simplifyByDistance(closed, distanceDiff);
    simplifyByAngle(closed, angleDiff);
    angleDiff += 0.5;
    distanceDiff *= 2;
    if (angleDiff >= halfCircle) {
      break;
    }
    if (distanceDiff >= bboxSmallest) {
      break;
    }
  }
}

// ____________________________________________________________________________
osm2ttl::osm::SimplifyingWKTFactoryImpl::point_type
osm2ttl::osm::SimplifyingWKTFactoryImpl::make_point(
    const osmium::geom::Coordinates& xy) const {
  std::string str{"POINT"};
  xy.append_to_string(str, '(', ' ', ')', _precision);
  return str;
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::linestring_start() {
  _coordinates.clear();
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::linestring_add_location(
  const osmium::geom::Coordinates& xy) {
  _coordinates.push_back(xy);
}

// ____________________________________________________________________________
osm2ttl::osm::SimplifyingWKTFactoryImpl::linestring_type
osm2ttl::osm::SimplifyingWKTFactoryImpl::linestring_finish(size_t /*unused*/) {
  std::string str{"LINESTRING("};
  simplify(false);
  for (const osmium::geom::Coordinates& c : _coordinates) {
    c.append_to_string(str, ' ', _precision);
    str += ',';
  }
  str.back() = ')';
  return str;
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::polygon_start() {
  _coordinates.clear();
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::polygon_add_location(
  const osmium::geom::Coordinates& xy) {
  _coordinates.push_back(xy);
}

// ____________________________________________________________________________
osm2ttl::osm::SimplifyingWKTFactoryImpl::polygon_type
osm2ttl::osm::SimplifyingWKTFactoryImpl::polygon_finish(size_t /*unused*/) {
  std::string str{"POLYGON(("};
  simplify(true);
  for (const osmium::geom::Coordinates& c : _coordinates) {
    c.append_to_string(str, ' ', _precision);
    str += ',';
  }
  str.back() = ')';
  str += ')';
  return str;
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_start() {
  _buffer.clear();
  _buffer = "MULTIPOLYGON(";
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_add_location(
  const osmium::geom::Coordinates& xy) {
  _coordinates.push_back(xy);
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_inner_ring_start() {
  _coordinates.clear();
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_inner_ring_finish() {
  simplify(true);
  _buffer += ",(";
  for (const osmium::geom::Coordinates& c : _coordinates) {
    c.append_to_string(_buffer, ' ', _precision);
    _buffer += ',';
  }
  _buffer.back() = ')';
  _buffer += ',';
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_outer_ring_start() {
  _coordinates.clear();
  _buffer += '(';
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_outer_ring_finish() {
  simplify(true);
  for (const osmium::geom::Coordinates& c : _coordinates) {
    c.append_to_string(_buffer, ' ', _precision);
    _buffer += ',';
  }
  _buffer.back() = ')';
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_polygon_start() {
  _buffer += '(';
}

// ____________________________________________________________________________
void osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_polygon_finish() {
  _buffer += "),";
}

// ____________________________________________________________________________
osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_type
osm2ttl::osm::SimplifyingWKTFactoryImpl::multipolygon_finish() {
  _buffer.back() = ')';
  std::string str;
  std::swap(str, _buffer);
  return str;
}
