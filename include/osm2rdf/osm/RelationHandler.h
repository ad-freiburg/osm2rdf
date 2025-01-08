// Copyright 2022, University of Freiburg
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

#ifndef OSM2RDF_OSM_RELATIONHANDLER_H
#define OSM2RDF_OSM_RELATIONHANDLER_H

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/LocationHandler.h"

namespace osm2rdf::osm {

class RelationHandler : public osmium::handler::Handler {
 public:
  explicit RelationHandler(const osm2rdf::config::Config& config);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);
  void prepare_for_lookup();
  void setLocationHandler(osm2rdf::osm::LocationHandler* locationHandler);
  bool hasLocationHandler() const;
  osmium::Location get_node_location(const uint64_t nodeId) const;
  std::vector<uint64_t> get_noderefs_of_way(const uint64_t wayId);

 private:
  std::vector<uint32_t> getCompressedIDs(const osmium::Way& way) const;
  std::vector<uint64_t> getNodeRefs(const std::vector<uint32_t>& refs) const;

 protected:
  osm2rdf::config::Config _config;
  osm2rdf::osm::LocationHandler* _locationHandler = nullptr;
  std::unordered_map<uint32_t, std::vector<uint32_t>> _ways32;
  std::unordered_map<uint64_t, std::vector<uint32_t>> _ways64;
  bool _firstPassDone = false;
};
}

#endif  // OSM2RDF_OSM_RELATIONHANDLER_H
