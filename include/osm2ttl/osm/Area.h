// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREA_H_
#define OSM2TTL_OSM_AREA_H_

static const int Base10Base = 10;
#include <utility>
#include <vector>

#include "boost/geometry/geometries/geometries.hpp"
#include "boost/serialization/nvp.hpp"
#include "osm2ttl/geometry/Area.h"
#include "osm2ttl/geometry/Global.h"
#include "osm2ttl/osm/Box.h"
#include "osmium/osm/area.hpp"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"

namespace osm2ttl::osm {

struct Area {
  typedef uint32_t id_t;

  Area();
  explicit Area(const osmium::Area& area);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] id_t objId() const noexcept;

  [[nodiscard]] osm2ttl::geometry::Area geom() const noexcept;
  [[nodiscard]] osm2ttl::geometry::area_result_t geomArea() const noexcept;
  [[nodiscard]] osm2ttl::geometry::Box envelope() const noexcept;
  [[nodiscard]] osm2ttl::geometry::area_result_t envelopeArea() const noexcept;
  [[nodiscard]] char tagAdministrationLevel() const noexcept;
  [[nodiscard]] bool fromWay() const noexcept;
  [[nodiscard]] bool hasName() const noexcept;

  bool operator==(const osm2ttl::osm::Area& other) const noexcept;
  bool operator!=(const osm2ttl::osm::Area& other) const noexcept;
  bool operator<(const osm2ttl::osm::Area& other) const noexcept;

 protected:
  id_t _id;
  id_t _objId;
  char _tagAdministrationLevel;
  bool _hasName;
  osm2ttl::geometry::area_result_t _geomArea;
  osm2ttl::geometry::area_result_t _envelopeArea;
  osm2ttl::geometry::Area _geom;
  osm2ttl::geometry::Box _envelope;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, [[maybe_unused]] const unsigned int version) {
    ar& boost::serialization::make_nvp("_id", _id);
    ar& boost::serialization::make_nvp("_objId", _objId);
    ar& boost::serialization::make_nvp("_tagAdministrationLevel",
                                       _tagAdministrationLevel);
    ar& boost::serialization::make_nvp("_hasName", _hasName);
    ar& boost::serialization::make_nvp("_geomArea", _geomArea);
    ar& boost::serialization::make_nvp("_envelopeArea", _envelopeArea);
    ar& boost::serialization::make_nvp("_geom", _geom);
    ar& boost::serialization::make_nvp("_envelope", _envelope);
  }
};

}  // namespace osm2ttl::osm

#endif  // OSM2TTL_OSM_AREA_H_
