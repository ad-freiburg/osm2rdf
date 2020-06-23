// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_AREASTACK_H_
#define OSM2TTL_OSM_AREASTACK_H_

#include <vector>

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/AreaHandler.h"

namespace osm2ttl {
namespace osm {

class AreaStack {
 public:
  explicit AreaStack(osm2ttl::osm::AreaHandler* areaHandler);
  void add(const osm2ttl::osm::Area& area);
  void add(uint64_t area);
  osm2ttl::osm::Area lookup(uint64_t areaId);
  void sort();
 protected:
  std::vector<uint64_t> elements;
  osm2ttl::osm::AreaHandler* _areaHandler;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_AREASTACK_H_
