// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Area.h"

#include <iostream>
#include <vector>
#include <utility>

#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"

#include "osm2ttl/osm/Ring.h"

// ____________________________________________________________________________
osm2ttl::osm::Area::Area(const osmium::Area& area) {
  _id = area.positive_id();
  _box = area.envelope();

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
uint64_t osm2ttl::osm::Area::id() const {
  return _id;
}

// ____________________________________________________________________________
uint64_t osm2ttl::osm::Area::objId() const {
  return _objId;
}

// ____________________________________________________________________________
osmium::Box osm2ttl::osm::Area::bbox() const {
  return _box;
}

// ____________________________________________________________________________
char osm2ttl::osm::Area::tagAdministrationLevel() const {
  return _tagAdministrationLevel;
}

// ____________________________________________________________________________
std::vector<osm2ttl::osm::OuterRing> osm2ttl::osm::Area::rings() const {
  return _rings;
}

// ____________________________________________________________________________
double osm2ttl::osm::Area::vagueArea() const {
  std::cout << _box.top_right().x() << "-" << _box.bottom_left().x()
    << "=" << _box.top_right().x() - _box.bottom_left().x() << "\n";
  std::cout << _box.top_right().y() << "-" << _box.bottom_left().y()
    << "=" << _box.top_right().y() - _box.bottom_left().y() << "\n";
  return (_box.top_right().x() - _box.bottom_left().x()) *
         (_box.top_right().y() - _box.bottom_left().y());
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::vagueIntersects(const osm2ttl::osm::Area& other)
  const {
  return _box.contains(osmium::Location(other._box.bottom_left().x(),
                                        other._box.bottom_left().y()))
      || _box.contains(osmium::Location(other._box.bottom_left().x(),
                                        other._box.top_right().y()))
      || _box.contains(osmium::Location(other._box.top_right().x(),
                                        other._box.top_right().y()))
      || _box.contains(osmium::Location(other._box.top_right().x(),
                                        other._box.bottom_left().y()));
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::vagueContains(const osm2ttl::osm::Area& other)
  const {
  return _box.contains(other._box.bottom_left())
      && _box.contains(other._box.top_right());
}

// ____________________________________________________________________________
double osm2ttl::osm::Area::area() const {
  double res = 0.0;
  for (const auto& outer : _rings) {
    res += osm2ttl::osm::Area::area(outer);
    for (const auto& inner : outer.inner) {
      res -= osm2ttl::osm::Area::area(inner);
    }
  }
  return res;
}

// ____________________________________________________________________________
double osm2ttl::osm::Area::area(const osm2ttl::osm::Ring& ring) const {
  double res = 0.0;
  for (size_t i = 0; i < ring.vertices.size() - 1; ++i) {
    const osmium::Location& l1 = ring.vertices[i];
    const osmium::Location& l2 = ring.vertices[i + 1];
    std::cout << "  " << l1.x() << " " << l1.y() << "\n";
    std::cout << "  " << l2.x() << " " << l2.y() << "\n";
    res += 0.5 * (l1.x() * l2.y()) - (l1.y() * l2.x());
    std::cout << "|" << l1 << " " << l2 << " :: " << res << "\n";
  }
  return res;
}


// ____________________________________________________________________________
bool osm2ttl::osm::Area::intersects(const osm2ttl::osm::Area& other)
  const {
  return false;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::contains(const osm2ttl::osm::Area& other) const {
  return false;
}
