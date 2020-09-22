// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_WAY_H_
#define OSM2TTL_OSM_WAY_H_

#include <vector>

#include "osmium/osm/way.hpp"

#include "osm2ttl/geometry/Linestring.h"
#include "osm2ttl/geometry/Location.h"

#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/TagList.h"

namespace osm2ttl {
namespace osm {

class Way {
 public:
  Way();
  explicit Way(const osmium::Way& way);
  [[nodiscard]] uint64_t id() const noexcept;
  [[nodiscard]] bool closed() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Linestring geom() const noexcept;
  [[nodiscard]] std::vector<osm2ttl::osm::Node> nodes() const noexcept;
  [[nodiscard]] osm2ttl::osm::TagList tags() const noexcept;

  bool operator==(const osm2ttl::osm::Way& other) const;
  bool operator<(const osm2ttl::osm::Way& other) const;
 protected:
  uint64_t _id;
  std::vector<osm2ttl::osm::Node> _nodes;
  osm2ttl::geometry::Linestring _geom;
  osm2ttl::geometry::Box _envelope;
  osm2ttl::osm::TagList _tags;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_WAY_H_

