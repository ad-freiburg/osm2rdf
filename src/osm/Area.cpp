// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Area.h"

#include <iostream>
#include <limits>
#include <vector>

#include "boost/geometry.hpp"
#include "osmium/osm/area.hpp"

#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/geometry/Box.h"
#include "osm2ttl/geometry/Location.h"

#include "osm2ttl/osm/Box.h"

// ____________________________________________________________________________
osm2ttl::osm::Area::Area() {
  _id = std::numeric_limits<uint64_t>::max();
  _objId = std::numeric_limits<uint64_t>::max();
}
// ____________________________________________________________________________
osm2ttl::osm::Area::Area(const osmium::Area& area) {
  _id = area.positive_id();
  _objId = area.orig_id();
  if (area.tags()["boundary"] != nullptr
      && area.tags()["admin_level"] != nullptr) {
    _tagAdministrationLevel = atoi(area.tags()["admin_level"]);
  }

  auto outerRings = area.outer_rings();
  _geom.resize(outerRings.size());
  size_t oCount = 0;
  for (const auto& oring : outerRings) {
    _geom[oCount].outer().reserve(oring.size());
    for (const auto& nodeRef : oring) {
      auto l = nodeRef.location();
      boost::geometry::append(_geom,
                              osm2ttl::geometry::Location(l.lon(), l.lat()), -1, oCount);
    }

    auto innerRings = area.inner_rings(oring);
    _geom[oCount].inners().resize(innerRings.size());
    size_t iCount = 0;
    for (const auto& iring : innerRings) {
      _geom[oCount].inners()[iCount].reserve(iring.size());
      for (const auto& nodeRef : iring) {
        auto l = nodeRef.location();
        boost::geometry::append(_geom,
                                osm2ttl::geometry::Location(l.lon(), l.lat()), iCount, oCount);
      }
      iCount++;
    }
    oCount++;
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
osm2ttl::geometry::Area osm2ttl::osm::Area::geom() const {
  return _geom;
}

// ____________________________________________________________________________
osm2ttl::osm::Box osm2ttl::osm::Area::envelope() const noexcept {
  osm2ttl::geometry::Box box;
  boost::geometry::envelope(geom(), box);
  return osm2ttl::osm::Box(box);
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
bool osm2ttl::osm::Area::operator==(const osm2ttl::osm::Area& other) const {
  return _id == other._id;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::operator<(const osm2ttl::osm::Area& other) const {
  // If administration level is different, higher first
  if (_tagAdministrationLevel != other._tagAdministrationLevel) {
    return _tagAdministrationLevel > other._tagAdministrationLevel;
  }
  // Sort by area, smaller first
  auto ownArea = boost::geometry::area(envelope().geom());
  auto otherArea = boost::geometry::area(other.envelope().geom());
  if (ownArea != otherArea) {
    return ownArea < otherArea;
  }
  // No better metric -> sort by id, smaller first
  return _id < other._id;
}
