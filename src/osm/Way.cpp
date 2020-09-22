// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Way.h"

#include <vector>

#include "boost/geometry.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/geometry/Location.h"

#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/TagList.h"

// ____________________________________________________________________________
osm2ttl::osm::Way::Way() {
  _id = std::numeric_limits<uint64_t>::max();
}

// ____________________________________________________________________________
osm2ttl::osm::Way::Way(const osmium::Way& way) {
  _id = way.positive_id();
  _tags = osm2ttl::osm::convertTagList(way.tags());
  _nodes.reserve(way.nodes().size());
  for (const auto& nodeRef : way.nodes()) {
    _nodes.emplace_back(nodeRef);
    auto loc = nodeRef.location();
    boost::geometry::append(_geom, osm2ttl::geometry::Location(loc.lon(), loc.lat()));
  }
  boost::geometry::unique(_geom);
  boost::geometry::envelope(_geom, _envelope);
}

// ____________________________________________________________________________
uint64_t osm2ttl::osm::Way::id() const noexcept {
  return _id;
}

// ____________________________________________________________________________
osm2ttl::osm::TagList osm2ttl::osm::Way::tags() const noexcept {
  return _tags;
}

// ____________________________________________________________________________
std::vector<osm2ttl::osm::Node> osm2ttl::osm::Way::nodes() const noexcept {
  return _nodes;
}

// ____________________________________________________________________________
osm2ttl::geometry::Linestring osm2ttl::osm::Way::geom() const
noexcept {
  return _geom;
}

// ____________________________________________________________________________
osm2ttl::geometry::Box osm2ttl::osm::Way::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::closed() const noexcept {
  return boost::geometry::equals(_nodes.front().geom(), _nodes.back().geom());
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::operator==(const osm2ttl::osm::Way& other) const {
  return _id == other._id;
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::operator<(const osm2ttl::osm::Way& other) const {
  return _id < other._id;
}
