// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_SIMPLIFYINGWKTFACTORY_H_
#define OSM2TTL_OSM_SIMPLIFYINGWKTFACTORY_H_

#include <string>
#include <vector>

#include "osmium/geom/coordinates.hpp"
#include "osmium/geom/factory.hpp"

#include "osm2ttl/osm/MergeMode.h"

namespace osm2ttl {
namespace osm {

class SimplifyingWKTFactoryImpl {
 public:
  using linestring_type = std::string;
  using multipolygon_type = std::string;
  using point_type = std::string;
  using polygon_type = std::string;
  using ring_type = std::string;

  explicit SimplifyingWKTFactoryImpl(int, int precision = 7);

  point_type make_point(const osmium::geom::Coordinates& xy) const;

  void linestring_start();
  void linestring_add_location(const osmium::geom::Coordinates& xy);
  linestring_type linestring_finish(size_t);

  void polygon_start();
  void polygon_add_location(const osmium::geom::Coordinates& xy);
  polygon_type polygon_finish(size_t);

  void multipolygon_start();
  void multipolygon_add_location(const osmium::geom::Coordinates& xy);
  void multipolygon_inner_ring_start();
  void multipolygon_inner_ring_finish();
  void multipolygon_outer_ring_start();
  void multipolygon_outer_ring_finish();
  void multipolygon_polygon_start();
  void multipolygon_polygon_finish();
  multipolygon_type multipolygon_finish();

 protected:
  static double getDistance(const osmium::geom::Coordinates& x,
                            const osmium::geom::Coordinates& y);
  static double getAngle(const osmium::geom::Coordinates& a,
                         const osmium::geom::Coordinates& b,
                         const osmium::geom::Coordinates& c);
  void simplify(const bool closed);
  void simplifyByAngle(const bool closed, const double angleDiff);
  void simplifyByDistance(const bool closed, const double distanceDiff);
  void merge(const size_t index1, const size_t index2);
  std::vector<osmium::geom::Coordinates> _coordinates;
  std::string _buffer;
  int _precision = 7;
  size_t _maxCoordinates = 10;
  osm2ttl::osm::MergeMode _mergeMode = osm2ttl::osm::MergeMode::MERGE;
};

template <typename TProjection = osmium::geom::IdentityProjection>
using SimplifyingWKTFactory = osmium::geom::GeometryFactory<
  osm2ttl::osm::SimplifyingWKTFactoryImpl, TProjection>;

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_SIMPLIFYINGWKTFACTORY_H_
