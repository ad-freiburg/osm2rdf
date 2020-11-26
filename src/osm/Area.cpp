// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Area.h"

#include <iostream>
#include <limits>

#include "boost/geometry.hpp"
#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/geometry/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osmium/osm/area.hpp"

// ____________________________________________________________________________
osm2ttl::osm::Area::Area() {
  _id = std::numeric_limits<osm2ttl::osm::Area::id_t>::max();
  _objId = std::numeric_limits<osm2ttl::osm::Area::id_t>::max();
  _tagAdministrationLevel = 0;
}

// ____________________________________________________________________________
osm2ttl::osm::Area::Area(const osmium::Area& area) : Area() {
  _id = area.positive_id();
  _objId = static_cast<osm2ttl::osm::Area::id_t>(area.orig_id());
  if (area.tags()["boundary"] != nullptr &&
      area.tags()["admin_level"] != nullptr) {
    _tagAdministrationLevel = static_cast<char>(
        strtol(area.tags()["admin_level"], nullptr, Base10Base));
  }
  if (area.tags()["name"] != nullptr) {
    _hasName = true;
  }

  auto outerRings = area.outer_rings();
  _geom.resize(outerRings.size());
  // int and not size_t as boost uses int internal
  int oCount = 0;
  for (const auto& oring : outerRings) {
    _geom[oCount].outer().reserve(oring.size());
    for (const auto& nodeRef : oring) {
      boost::geometry::append(_geom, osm2ttl::osm::Node(nodeRef).geom(), -1,
                              oCount);
    }

    auto innerRings = area.inner_rings(oring);
    _geom[oCount].inners().resize(innerRings.size());
    // int and not size_t as boost uses int internal
    int iCount = 0;
    for (const auto& iring : innerRings) {
      _geom[oCount].inners()[iCount].reserve(iring.size());
      for (const auto& nodeRef : iring) {
        boost::geometry::append(_geom, osm2ttl::osm::Node(nodeRef).geom(),
                                iCount, oCount);
      }
      iCount++;
    }
    oCount++;
  }
  // Correct possibly invalid geometry...
  boost::geometry::correct(_geom);
  boost::geometry::envelope(_geom, _envelope);
  _geomArea = boost::geometry::area(_geom);
  _envelopeArea = boost::geometry::area(_envelope);
  assert(_geomArea > 0);
  assert(_envelopeArea > 0);
}

// ____________________________________________________________________________
osm2ttl::osm::Area::id_t osm2ttl::osm::Area::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2ttl::osm::Area::id_t osm2ttl::osm::Area::objId() const noexcept {
  return _objId;
}

// ____________________________________________________________________________
osm2ttl::geometry::Area osm2ttl::osm::Area::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
osm2ttl::geometry::Box osm2ttl::osm::Area::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
osm2ttl::osm::Area::area_t osm2ttl::osm::Area::geomArea() const noexcept {
  return _geomArea;
}

// ____________________________________________________________________________
osm2ttl::osm::Area::area_t osm2ttl::osm::Area::envelopeArea() const noexcept {
  return _envelopeArea;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Area::hasName() const noexcept { return _hasName; }

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
  auto ownArea = boost::geometry::area(envelope());
  auto otherArea = boost::geometry::area(other.envelope());
  if (ownArea != otherArea) {
    return ownArea < otherArea;
  }
  // No better metric -> sort by id, smaller first
  return _id < other._id;
}
