// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#include "osm2rdf/osm/Way.h"

#include <vector>

#include "osm2rdf/osm/Box.h"
#include "osmium/osm/way.hpp"

// ____________________________________________________________________________
osm2rdf::osm::Way::Way() {}

// ____________________________________________________________________________
osm2rdf::osm::Way::Way(const osmium::Way& way) : _w(&way) {}

// ____________________________________________________________________________
osm2rdf::osm::Way::id_t osm2rdf::osm::Way::id() const noexcept {
  if (!_w) return std::numeric_limits<osm2rdf::osm::Way::id_t>::max();
  return _w->id();
}

// ____________________________________________________________________________
osm2rdf::osm::generic::changeset_id_t osm2rdf::osm::Way::changeset()
    const noexcept {
  return _w->changeset();
}

// ____________________________________________________________________________
std::time_t osm2rdf::osm::Way::timestamp() const noexcept {
  return _w->timestamp().seconds_since_epoch();
}

// ____________________________________________________________________________
std::string osm2rdf::osm::Way::user() const noexcept { return _w->user(); }

// ____________________________________________________________________________
id_t osm2rdf::osm::Way::uid() const noexcept { return _w->uid(); }

// ____________________________________________________________________________
osm2rdf::osm::generic::version_t osm2rdf::osm::Way::version() const noexcept {
  return _w->version();
}
// ____________________________________________________________________________
bool osm2rdf::osm::Way::visible() const noexcept { return _w->visible(); }

// ____________________________________________________________________________
const osmium::TagList& osm2rdf::osm::Way::tags() const noexcept {
  return _w->tags();
}

// ____________________________________________________________________________
const osmium::WayNodeList& osm2rdf::osm::Way::nodes() const noexcept {
  return _w->nodes();
}

// ____________________________________________________________________________
const ::util::geo::DLine osm2rdf::osm::Way::geom() const noexcept {
  ::util::geo::DLine ret;
  for (const auto& nodeRef : _w->nodes()) {
    if (!nodeRef.location().valid()) continue;
    if (ret.empty() ||
        (nodeRef.location().lon_without_check() != ret.back().getX() ||
         nodeRef.location().lat_without_check() != ret.back().getY())) {
      ret.push_back({nodeRef.location().lon_without_check(),
                     nodeRef.location().lat_without_check()});
    }
  }
  return ret;
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::closed() const noexcept {
  return _w->nodes().size() > 1 && _w->nodes().front().location().valid() &&
         _w->nodes().back().location().valid() &&
         _w->nodes().front().location().lon_without_check() ==
             _w->nodes().back().location().lon_without_check() &&
         _w->nodes().front().location().lat_without_check() ==
             _w->nodes().back().location().lat_without_check();
}

// ____________________________________________________________________________
bool osm2rdf::osm::Way::isArea() const noexcept {
  // See libosmium/include/osmium/area/multipolygon_manager.hpp:154
  if (_w->nodes().size() < 4) {
    return false;
  }
  if (!closed()) {
    return false;
  }
  auto areaTag = _w->tags()["area"];
  return areaTag == nullptr || strcmp(areaTag, "no") != 0;
}
