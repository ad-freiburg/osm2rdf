// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_WAY_H_
#define OSM2TTL_OSM_WAY_H_

#include <vector>

#include "boost/serialization/nvp.hpp"
#include "boost/serialization/unordered_map.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2ttl/geometry/Location.h"
#include "osm2ttl/geometry/Way.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/TagList.h"
#include "osmium/osm/way.hpp"

namespace osm2ttl::osm {

class Way {
 public:
  typedef uint32_t id_t;
  Way();
  explicit Way(const osmium::Way& way);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] bool closed() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Way geom() const noexcept;
  [[nodiscard]] std::vector<osm2ttl::osm::Node> nodes() const noexcept;
  [[nodiscard]] osm2ttl::osm::TagList tags() const noexcept;

  bool operator==(const osm2ttl::osm::Way& other) const noexcept;
  bool operator!=(const osm2ttl::osm::Way& other) const noexcept;

 protected:
  id_t _id;
  std::vector<osm2ttl::osm::Node> _nodes;
  osm2ttl::geometry::Way _geom;
  osm2ttl::geometry::Box _envelope;
  osm2ttl::osm::TagList _tags;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_nodes", _nodes);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_envelope", _envelope);
    ar& boost::serialization::make_nvp("_tags", _tags);
  }
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_WAY_H_
