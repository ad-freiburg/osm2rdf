// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Node.h"

#include "boost/geometry/algorithms/envelope.hpp"
#include "osm2ttl/geometry/Box.h"
#include "osm2ttl/geometry/Global.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"

// ____________________________________________________________________________
osm2ttl::osm::Node::Node(const osmium::Node& node) {
  _id = node.positive_id();
  const auto& loc = node.location();
  if constexpr (std::is_integral<
                    osm2ttl::geometry::location_coordinate_t>::value) {
    _geom = osm2ttl::geometry::Location(loc.x(), loc.y());
  } else {
    _geom = osm2ttl::geometry::Location(loc.lon(), loc.lat());
  }
  boost::geometry::envelope(_geom, _envelope);
  _tags = osm2ttl::osm::convertTagList(node.tags());
}

// ____________________________________________________________________________
osm2ttl::osm::Node::Node(const osmium::NodeRef& nodeRef) {
  _id = nodeRef.positive_ref();
  const auto& loc = nodeRef.location();
  if constexpr (std::is_integral<
                    osm2ttl::geometry::location_coordinate_t>::value) {
    _geom = osm2ttl::geometry::Location(loc.x(), loc.y());
  } else {
    _geom = osm2ttl::geometry::Location(loc.lon(), loc.lat());
  }
  boost::geometry::envelope(_geom, _envelope);
}

// ____________________________________________________________________________
osm2ttl::osm::Node::id_t osm2ttl::osm::Node::id() const noexcept { return _id; }

// ____________________________________________________________________________
osm2ttl::geometry::Box osm2ttl::osm::Node::envelope() const noexcept {
  return _envelope;
}

// ____________________________________________________________________________
osm2ttl::geometry::Location osm2ttl::osm::Node::geom() const noexcept {
  return _geom;
}

// ____________________________________________________________________________
osm2ttl::osm::TagList osm2ttl::osm::Node::tags() const noexcept {
  return _tags;
}
