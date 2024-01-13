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

#ifndef OSM2RDF_OSM_WAY_H_
#define OSM2RDF_OSM_WAY_H_

#include <vector>

#include "boost/version.hpp"
#if BOOST_VERSION >= 107400 && BOOST_VERSION < 107500
#include "boost/serialization/library_version_type.hpp"
#endif
#include "boost/serialization/nvp.hpp"
#include "boost/serialization/unordered_map.hpp"
#include "boost/serialization/vector.hpp"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/geometry/Polygon.h"
#include "osm2rdf/geometry/Way.h"
#include "osm2rdf/osm/Box.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/way.hpp"

namespace osm2rdf::osm {

class Way {
 public:
  typedef uint32_t id_t;
  Way();
  explicit Way(const osmium::Way& way);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] bool closed() const noexcept;
  [[nodiscard]] bool isArea() const noexcept;
  [[nodiscard]] const osm2rdf::geometry::Box& envelope() const noexcept;
  [[nodiscard]] const osm2rdf::geometry::Way& geom() const noexcept;
  // Return the convex hull.
  [[nodiscard]] const osm2rdf::geometry::Polygon& convexHull() const noexcept;
  // Return the oriented bounding box.
  [[nodiscard]] const osm2rdf::geometry::Polygon& orientedBoundingBox() const noexcept;
  [[nodiscard]] const std::vector<osm2rdf::osm::Node>& nodes() const noexcept;
  [[nodiscard]] const osm2rdf::osm::TagList& tags() const noexcept;

  bool operator==(const osm2rdf::osm::Way& other) const noexcept;
  bool operator!=(const osm2rdf::osm::Way& other) const noexcept;

 protected:
  id_t _id;
  std::vector<osm2rdf::osm::Node> _nodes;
  osm2rdf::geometry::Way _geom;
  osm2rdf::geometry::Box _envelope;
  osm2rdf::geometry::Polygon _convexHull;
  osm2rdf::geometry::Polygon _obb;
  osm2rdf::osm::TagList _tags;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_nodes", _nodes);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_envelope", _envelope);
    ar& boost::serialization::make_nvp("_convexHull", _envelope);
    ar& boost::serialization::make_nvp("_obb", _geom);
    ar& boost::serialization::make_nvp("_tags", _tags);
  }
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_WAY_H_
