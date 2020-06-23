// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Area.h"

#include <iostream>
#include <vector>
#include <utility>

#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/location.hpp"

#include "osm2ttl/osm/Ring.h"

// ____________________________________________________________________________
osm2ttl::osm::Area::Area(const osmium::Area& area) {
  _id = area.positive_id();
  _objId = area.orig_id();

  for (const auto& ring : area.outer_rings()) {
    _rings.emplace_back();
    for (const auto& noderef : ring) {
      _rings.back().vertices.push_back(noderef.location());
    }

    for (const auto& iring : area.inner_rings(ring)) {
      _rings.back().inner.emplace_back();
      for (const auto& noderef : iring) {
      _rings.back().inner.back().vertices.push_back(noderef.location());
      }
    }
  }
}

// ____________________________________________________________________________
uint64_t osm2ttl::osm::Area::id() const noexcept {
  return _id;
}

// ____________________________________________________________________________
uint64_t osm2ttl::osm::Area::objId() const noexcept {
  return _objId;
}

// ____________________________________________________________________________
osmium::Box osm2ttl::osm::Area::bbox() const noexcept {
  return _rings[0].bbox();
}

// ____________________________________________________________________________
osmium::Location osm2ttl::osm::Area::centroid() const noexcept {
  return _rings[0].centroid();
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::fromWay() const noexcept {
  // https://github.com/osmcode/libosmium/blob/master/include/osmium/osm/area.hpp#L145-L153
  return (_id & 0x1U) == 0;
}

// ____________________________________________________________________________
char osm2ttl::osm::Area::tagAdministrationLevel() const noexcept {
  return _tagAdministrationLevel;
}

// ____________________________________________________________________________
std::vector<osm2ttl::osm::OuterRing> osm2ttl::osm::Area::rings() const
  noexcept {
  return _rings;
}

// ____________________________________________________________________________
double osm2ttl::osm::Area::vagueArea() const noexcept {
  const osmium::Location tr = bbox().top_right();
  const osmium::Location bl = bbox().bottom_left();
  return std::abs((tr.x() - bl.x()) * (tr.y() - bl.y()));
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::vagueIntersects(const osm2ttl::osm::Area& other)
  const noexcept {
  return bbox().contains(osmium::Location(other.bbox().bottom_left().x(),
                                        other.bbox().bottom_left().y()))
      || bbox().contains(osmium::Location(other.bbox().bottom_left().x(),
                                        other.bbox().top_right().y()))
      || bbox().contains(osmium::Location(other.bbox().top_right().x(),
                                        other.bbox().top_right().y()))
      || bbox().contains(osmium::Location(other.bbox().top_right().x(),
                                        other.bbox().bottom_left().y()));
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::vagueContains(const osm2ttl::osm::Area& other)
  const noexcept {
  return bbox().contains(other.bbox().bottom_left())
      && bbox().contains(other.bbox().top_right());
}

// ____________________________________________________________________________
double osm2ttl::osm::Area::area() const noexcept {
  double res = 0.0;
  for (const auto& outer : _rings) {
    res += outer.area();
    for (const auto& inner : outer.inner) {
      res -= inner.area();
    }
  }
  return res;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::intersects(const osm2ttl::osm::Area& other)
  const noexcept {
  return false;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::contains(const osm2ttl::osm::Area& other) const
  noexcept {
  return false;
}
