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
osm2ttl::osm::Way::Way(const osmium::Way& way) {
  _id = way.positive_id();
  _tags = osm2ttl::osm::convertTagList(way.tags());
  _nodes.reserve(way.nodes().size());
  for (const auto& noderef : way.nodes()) {
    _nodes.emplace_back(noderef);
  }
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
  osm2ttl::geometry::Linestring locations;
  for (const auto& node : _nodes) {
    boost::geometry::append(locations, node.geom());
  }
  boost::geometry::unique(locations);
  return locations;
}

// ____________________________________________________________________________
osm2ttl::osm::Box osm2ttl::osm::Way::envelope() const noexcept {
  osm2ttl::geometry::Box box;
  boost::geometry::envelope(geom(), box);
  return osm2ttl::osm::Box(box);
}

// ____________________________________________________________________________
bool osm2ttl::osm::Way::closed() const noexcept {
  return boost::geometry::equals(_nodes.front().geom(), _nodes.back().geom());
}
