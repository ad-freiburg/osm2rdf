// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_NODE_H_
#define OSM2TTL_OSM_NODE_H_

#include "boost/serialization/nvp.hpp"
#include "osm2ttl/geometry/Box.h"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"

namespace osm2ttl::osm {

class Node {
 public:
  typedef uint64_t id_t;
  explicit Node();
  explicit Node(const osmium::Node& node);
  explicit Node(const osmium::NodeRef& nodeRef);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Location geom() const noexcept;
  [[nodiscard]] osm2ttl::osm::TagList tags() const noexcept;

  bool operator==(const osm2ttl::osm::Node& other) const noexcept;
  bool operator!=(const osm2ttl::osm::Node& other) const noexcept;

 protected:
  id_t _id;
  osm2ttl::geometry::Location _geom;
  osm2ttl::geometry::Box _envelope;
  osm2ttl::osm::TagList _tags;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_envelope", _envelope);
    ar& boost::serialization::make_nvp("_tags", _tags);
  }
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_NODE_H_
