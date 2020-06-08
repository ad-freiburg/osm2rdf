// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/osm/SimplifyingWKTFactory.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <utility>

// ____________________________________________________________________________
osm2nt::osm::SimplifyingWKTFactoryImpl::SimplifyingWKTFactoryImpl(
  int /*unused*/, int precision) {
  this->precision = precision;
}

// ____________________________________________________________________________
double osm2nt::osm::SimplifyingWKTFactoryImpl::getDistance(
  const osmium::geom::Coordinates& x, const osmium::geom::Coordinates& y) {
  const double a = x.x - y.x;
  const double b = x.y - y.y;
  return std::abs(std::sqrt(a*a + b*b));
}

// ____________________________________________________________________________
double osm2nt::osm::SimplifyingWKTFactoryImpl::getAngle(
  const osmium::geom::Coordinates& a, const osmium::geom::Coordinates& b,
  const osmium::geom::Coordinates& c) {
  return atan2(b.y - a.y, b.x - a.x) - atan2(c.y - a.y, c.x - a.x);
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::merge(const size_t index1,
  const size_t index2) {
  switch (mergeMode) {
  case DELETE_FIRST:
    coordinates.erase(coordinates.begin() + index1);
    break;
  case DELETE_SECOND:
    coordinates.erase(coordinates.begin() + index2);
    break;
  case MERGE:
    const double x = (coordinates[index1].x + coordinates[index2].x) / 2;
    const double y = (coordinates[index1].y + coordinates[index2].y) / 2;
    coordinates.erase(coordinates.begin() + index2);
    coordinates.erase(coordinates.begin() + index1);
    coordinates.emplace(coordinates.begin() + index1, x, y);
    break;
  }
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::simplifyByAngle(const bool closed,
  const double angleDiff) {
  const double halfCircle = 180.0;
  // Close -> handle indizes n-1 0 1
  if (closed) {
    while (true) {
      if (coordinates.size() < 3) {
        break;
      }
      const size_t pos0 = 0;
      const size_t pos1 = 1;
      const size_t pos2 = coordinates.size() - 1;
      const double angle = std::abs(halfCircle - getAngle(coordinates[pos0],
                                                          coordinates[pos1],
                                                          coordinates[pos2]));
      if (angle >= angleDiff) {
        break;
      }
      coordinates.erase(coordinates.begin() + pos0);
    }
  }
  // handle indizes 0 1 2, ... n-3 n-2 n-1
  for (size_t pos = 1; pos < (coordinates.size() - 2); ++pos) {
    while (true) {
      if (coordinates.size() < 3) {
        break;
      }
      const size_t pos0 = pos;
      const size_t pos1 = pos - 1;
      const size_t pos2 = pos + 1;
      const double angle = std::abs(halfCircle - getAngle(coordinates[pos0],
                                                          coordinates[pos1],
                                                          coordinates[pos2]));
      if (angle >= angleDiff) {
        break;
      }
      coordinates.erase(coordinates.begin() + pos0);
    }
  }
  // Close -> handle indizes n-2 n-1 0
  if (closed) {
    while (true) {
      if (coordinates.size() < 3) {
        break;
      }
      const size_t pos0 = coordinates.size() - 1;
      const size_t pos1 = coordinates.size() - 2;
      const size_t pos2 = 0;
      const double angle = std::abs(halfCircle - getAngle(coordinates[pos0],
                                                          coordinates[pos1],
                                                          coordinates[pos2]));
      if (angle >= angleDiff) {
        break;
      }
      coordinates.erase(coordinates.begin() + pos0);
    }
  }
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::simplifyByDistance(
  const bool closed, const double distanceDiff) {
  if (closed) {
    while (true) {
      if (coordinates.size() < 2) {
        break;
      }
      const size_t pos0 = 0;
      const size_t pos1 = coordinates.size() - 1;
      if (getDistance(coordinates[pos0], coordinates[pos1]) > distanceDiff) {
        break;
      }
      merge(pos0, pos1);
    }
  }
  for (size_t pos = 1; pos < (coordinates.size() - 1); ++pos) {
    while (true) {
      // Abort if not enough coordinates remaining or we moved outside the list.
      if (coordinates.size() < 2 || pos > (coordinates.size() - 1)) {
        break;
      }
      const size_t pos0 = pos - 1;
      const size_t pos1 = pos;
      if (getDistance(coordinates[pos0], coordinates[pos1]) > distanceDiff) {
        break;
      }
      merge(pos0, pos1);
    }
  }
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::simplify(const bool closed) {
  size_t oldCount = coordinates.size();
  double minX = coordinates[0].x;
  double minY = coordinates[0].y;
  double maxX = coordinates[0].x;
  double maxY = coordinates[0].y;
  for (const osmium::geom::Coordinates& c : coordinates) {
    minX = std::min(minX, c.x);
    minY = std::min(minY, c.y);
    maxX = std::max(maxX, c.x);
    maxY = std::max(maxY, c.y);
  }

  const double bboxSmallest = std::min(std::abs(maxX-minX),
                                       std::abs(maxY-minY));
  double distanceDiff = bboxSmallest / 1000000.0;
  const double halfCircle = 180.0;
  double angleDiff = 2;
  while (maxCoordinates > 0 && coordinates.size() > maxCoordinates) {
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
  if (coordinates.size() < oldCount) {
    std::cout << "Simplified " << oldCount << " to " << coordinates.size()
      << " nodes" << std::endl;
    std::cout << "AngleDiff " << angleDiff << " DistanceDiff " << distanceDiff
      << std::endl;
  }
}

// ____________________________________________________________________________
osm2nt::osm::SimplifyingWKTFactoryImpl::point_type
osm2nt::osm::SimplifyingWKTFactoryImpl::make_point(
    const osmium::geom::Coordinates& xy) const {
  std::string str{"POINT"};
  xy.append_to_string(str, '(', ' ', ')', precision);
  return str;
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::linestring_start() {
  coordinates.clear();
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::linestring_add_location(
  const osmium::geom::Coordinates& xy) {
  coordinates.push_back(xy);
}

// ____________________________________________________________________________
osm2nt::osm::SimplifyingWKTFactoryImpl::linestring_type
osm2nt::osm::SimplifyingWKTFactoryImpl::linestring_finish(size_t /*unused*/) {
  std::string str{"LINESTRING("};
  simplify(false);
  for (const osmium::geom::Coordinates& c : coordinates) {
    c.append_to_string(str, ' ', precision);
    str += ',';
  }
  str.back() = ')';
  return str;
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::polygon_start() {
  coordinates.clear();
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::polygon_add_location(
  const osmium::geom::Coordinates& xy) {
  coordinates.push_back(xy);
}

// ____________________________________________________________________________
osm2nt::osm::SimplifyingWKTFactoryImpl::polygon_type
osm2nt::osm::SimplifyingWKTFactoryImpl::polygon_finish(size_t /*unused*/) {
  std::string str{"POLYGON(("};
  simplify(true);
  for (const osmium::geom::Coordinates& c : coordinates) {
    c.append_to_string(str, ' ', precision);
    str += ',';
  }
  str.back() = ')';
  str += ')';
  return str;
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_start() {
  buffer.clear();
  buffer = "MULTIPOLYGON(";
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_add_location(
  const osmium::geom::Coordinates& xy) {
  coordinates.push_back(xy);
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_inner_ring_start() {
  coordinates.clear();
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_inner_ring_finish() {
  simplify(true);
  buffer += ",(";
  for (const osmium::geom::Coordinates& c : coordinates) {
    c.append_to_string(buffer, ' ', precision);
    buffer += ',';
  }
  buffer.back() = ')';
  buffer += ',';
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_outer_ring_start() {
  coordinates.clear();
  buffer += '(';
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_outer_ring_finish() {
  simplify(true);
  for (const osmium::geom::Coordinates& c : coordinates) {
    c.append_to_string(buffer, ' ', precision);
    buffer += ',';
  }
  buffer.back() = ')';
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_polygon_start() {
  buffer += '(';
}

// ____________________________________________________________________________
void osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_polygon_finish() {
  buffer += "),";
}

// ____________________________________________________________________________
osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_type
osm2nt::osm::SimplifyingWKTFactoryImpl::multipolygon_finish() {
  buffer.back() = ')';
  std::string str;
  std::swap(str, buffer);
  return str;
}
