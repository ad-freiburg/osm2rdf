// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Way.h"

#include <vector>

#include "boost/geometry.hpp"
#include "osm2ttl/geometry/Way.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/way.hpp"

// ____________________________________________________________________________
osm2ttl::osm::Way::Way() { _id = std::numeric_limits<uint64_t>::max(); }

// ____________________________________________________________________________
osm2ttl::osm::Way::Way(const osmium::Way& way) {
  _id = way.positive_id();
  _tags = osm2ttl::osm::convertTagList(way.tags());
  _nodes.reserve(way.nodes().size());
  for (const auto& nodeRef : way.nodes()) {
    _nodes.emplace_back(nodeRef);
    boost::geometry::append(_geom, osm2ttl::osm::Node(nodeRef).geom());
  }
  boost::geometry::unique(_geom);
  boost::geometry::envelope(_geom, _envelope);
}

// ____________________________________________________________________________
uint64_t osm2ttl::osm::Way::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2ttl::osm::TagList osm2ttl::osm::Way::tags() const noexcept { return _tags; }

// ____________________________________________________________________________
std::vector<osm2ttl::osm::Node> osm2ttl::osm::Way::nodes() const noexcept {
  return _nodes;
}

// ____________________________________________________________________________
osm2ttl::geometry::Way osm2ttl::osm::Way::geom() const noexcept {
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