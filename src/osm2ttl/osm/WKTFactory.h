// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_WKTFACTORY_H_
#define OSM2TTL_OSM_WKTFACTORY_H_

#include <string>

#include "osmium/geom/wkt.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/osm/SimplifyingWKTFactory.h"

namespace osm2ttl {
namespace osm {

class WKTFactory {
 public:
  virtual ~WKTFactory() {}
  virtual std::string create_point(const osmium::Location& location) = 0;
  virtual std::string create_point(const osmium::NodeRef& noderef) = 0;
  virtual std::string create_polygon(const osmium::Way& way) = 0;
  virtual std::string create_linestring(const osmium::Way& way) = 0;
  virtual std::string create_linestring(const osmium::Way& way,
                                        osmium::geom::use_nodes mode) = 0;
  virtual std::string create_multipolygon(const osmium::Area& area) = 0;

  // Helper creating the correct instance.
  static WKTFactory* create(const osm2ttl::config::Config& config);
};

template<class T>
class WKTFactoryImpl : public WKTFactory {
 public:
  std::string create_point(const osmium::Location& location);
  std::string create_point(const osmium::NodeRef& noderef);
  std::string create_polygon(const osmium::Way& way);
  std::string create_linestring(const osmium::Way& way);
  std::string create_linestring(const osmium::Way& way,
                                osmium::geom::use_nodes mode);
  std::string create_multipolygon(const osmium::Area& area);

 protected:
  T _factory;
};

using WKTOsmium = WKTFactoryImpl<osmium::geom::WKTFactory<>>;
using WKTSimplifying = WKTFactoryImpl<osm2ttl::osm::SimplifyingWKTFactory<>>;

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_WKTFACTORY_H_
