// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_OSM_ELEMENTSTACK_H_
#define OSM2NT_OSM_ELEMENTSTACK_H_

#include <vector>

#include "osm2nt/osm/Element.h"

namespace osm2nt {
namespace osm {

class ElementStack {
 public:
  void add(const osm2nt::osm::Element& element);
  void sort();
 protected:
  std::vector<osm2nt::osm::Element> elements;
  static bool _sort(const osm2nt::osm::Element& i,
                    const osm2nt::osm::Element& j);
};

}  // namespace osm
}  // namespace osm2nt

#endif  // OSM2NT_OSM_ELEMENTSTACK_H_
