// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREASTACK_H_
#define OSM2TTL_OSM_AREASTACK_H_

#include <vector>

#include "osm2ttl/osm/Area.h"

namespace osm2ttl {
namespace osm {

class AreaStack {
 public:
  void add(const osm2ttl::osm::Area& area);
  void sort();
 protected:
  std::vector<osm2ttl::osm::Area> elements;
  static bool _sort(const osm2ttl::osm::Area& i,
                    const osm2ttl::osm::Area& j);
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREASTACK_H_
