// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_ELEMENTSTACK_H_
#define OSM2TTL_OSM_ELEMENTSTACK_H_

#include <vector>

#include "osm2ttl/osm/Element.h"

namespace osm2ttl {
namespace osm {

class ElementStack {
 public:
  void add(const osm2ttl::osm::Element& element);
  void sort();
 protected:
  std::vector<osm2ttl::osm::Element> elements;
  static bool _sort(const osm2ttl::osm::Element& i,
                    const osm2ttl::osm::Element& j);
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_ELEMENTSTACK_H_
