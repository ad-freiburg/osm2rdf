// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_NODE_H_
#define OSM2TTL_OSM_NODE_H_

#include "osm2ttl/geometry/Box.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"

namespace osm2ttl {
namespace osm {

class Node {
 public:
  explicit Node(const osmium::Node& node);
  explicit Node(const osmium::NodeRef& nodeRef);
  [[nodiscard]] uint64_t id() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Location geom() const noexcept;
  [[nodiscard]] osm2ttl::osm::TagList tags() const noexcept;

 protected:
  uint64_t _id;
  osm2ttl::geometry::Location _geom;
  osm2ttl::geometry::Box _envelope;
  osm2ttl::osm::TagList _tags;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_NODE_H_
