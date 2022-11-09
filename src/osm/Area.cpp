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

#include <iostream>
#include <limits>

#include "boost/geometry.hpp"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Box.h"
#include "osm2rdf/geometry/Global.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/osm/Node.h"
#include "osmium/osm/area.hpp"

using osm2rdf::geometry::Location;

// ____________________________________________________________________________
osm2rdf::osm::Area::Area() {
  _id = std::numeric_limits<osm2rdf::osm::Area::id_t>::max();
  _objId = std::numeric_limits<osm2rdf::osm::Area::id_t>::max();
}

// ____________________________________________________________________________
osm2rdf::osm::Area::Area(const osmium::Area& area) : Area() {
  _id = area.positive_id();
  _objId = static_cast<osm2rdf::osm::Area::id_t>(area.orig_id());
  _hasName = (area.tags()["name"] != nullptr);

  const auto& outerRings = area.outer_rings();
  _geom.resize(outerRings.size());
  // int and not size_t as boost uses int internal
  int oCount = 0;
  for (const auto& oring : outerRings) {
    _geom[oCount].outer().reserve(oring.size());
    for (const auto& nodeRef : oring) {
      boost::geometry::append(_geom, Location{nodeRef.lon(), nodeRef.lat()},
                              -1, oCount);
    }

    const auto& innerRings = area.inner_rings(oring);
    _geom[oCount].inners().resize(innerRings.size());
    // int and not size_t as boost uses int internal
    int iCount = 0;
    for (const auto& iring : innerRings) {
      _geom[oCount].inners()[iCount].reserve(iring.size());
      for (const auto& nodeRef : iring) {
        boost::geometry::append(_geom, Location{nodeRef.lon(), nodeRef.lat()},
                                iCount, oCount);
      }
      iCount++;
    }
    oCount++;
  }

  // Correct possibly invalid geometry...
  boost::geometry::correct(_geom);
  boost::geometry::unique(_geom);
  boost::geometry::envelope(_geom, _envelope);
  _geomArea = boost::geometry::area(_geom);
  _envelopeArea = boost::geometry::area(_envelope);
  assert(_geomArea > 0);
  assert(_envelopeArea > 0);
}

// ____________________________________________________________________________
osm2rdf::osm::Area::id_t osm2rdf::osm::Area::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2rdf::osm::Area::id_t osm2rdf::osm::Area::objId() const noexcept {
  return _objId;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Area& osm2rdf::osm::Area::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
const osm2rdf::geometry::Box& osm2rdf::osm::Area::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
osm2rdf::geometry::area_result_t osm2rdf::osm::Area::geomArea() const noexcept {
  return _geomArea;
}

// ____________________________________________________________________________
osm2rdf::geometry::area_result_t osm2rdf::osm::Area::envelopeArea()
    const noexcept {
  return _envelopeArea;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Area::hasName() const noexcept { return _hasName; }

// ____________________________________________________________________________
bool osm2rdf::osm::Area::fromWay() const noexcept {
  // https://github.com/osmcode/libosmium/blob/master/include/osmium/osm/area.hpp#L145-L153
  return (_id & 0x1U) == 0;
}
