// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_OSM_DUMPHANDLER_H_
#define OSM2NT_OSM_DUMPHANDLER_H_

#include <ostream>

#include "osmium/handler.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2nt/nt/Writer.h"

namespace osm2nt {
namespace osm {

class DumpHandler : public osmium::handler::Handler {
 public:
  explicit DumpHandler(std::ostream* os);
  ~DumpHandler();
  void area(const osmium::Area& area);
  void node(const osmium::Node& node);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
 protected:
  osm2nt::nt::Writer* w;
  bool ignoreUnnamed = true;
};

}  // namespace osm
}  // namespace osm2nt

#endif  // OSM2NT_OSM_DUMPHANDLER_H_
