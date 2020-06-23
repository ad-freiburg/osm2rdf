// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/WKTFactory.h"

#include <string>

#include "osmium/geom/factory.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

// ____________________________________________________________________________
osm2ttl::osm::WKTFactory* osm2ttl::osm::WKTFactory::create(
  const osm2ttl::config::Config& config) {
  if (config.simplifyWKT > 0) {
    return new osm2ttl::osm::WKTSimplifying();
  }
  return new osm2ttl::osm::WKTOsmium();
}

// ____________________________________________________________________________
template<class T>
std::string osm2ttl::osm::WKTFactoryImpl<T>::create_point(
  const osmium::Location& location) {
  return _factory.create_point(location);
}

// ____________________________________________________________________________
template<class T>
std::string osm2ttl::osm::WKTFactoryImpl<T>::create_point(
  const osmium::NodeRef& noderef) {
  return _factory.create_point(noderef);
}

// ____________________________________________________________________________
template<class T>
std::string osm2ttl::osm::WKTFactoryImpl<T>::create_polygon(
  const osmium::Way& way) {
  return _factory.create_polygon(way);
}

// ____________________________________________________________________________
template<class T>
std::string osm2ttl::osm::WKTFactoryImpl<T>::create_linestring(
  const osmium::Way& way) {
  return _factory.create_linestring(way);
}

// ____________________________________________________________________________
template<class T>
std::string osm2ttl::osm::WKTFactoryImpl<T>::create_linestring(
  const osmium::Way& way, osmium::geom::use_nodes mode) {
  return _factory.create_linestring(way, mode);
}

// ____________________________________________________________________________
template<class T>
std::string osm2ttl::osm::WKTFactoryImpl<T>::create_multipolygon(
  const osmium::Area& area) {
  return _factory.create_multipolygon(area);
}
