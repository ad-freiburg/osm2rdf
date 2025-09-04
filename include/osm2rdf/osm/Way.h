// Copyright 2020, University of Freiburg
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

#ifndef OSM2RDF_OSM_WAY_H_
#define OSM2RDF_OSM_WAY_H_

#include <vector>

#include "osm2rdf/osm/Box.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/TagList.h"
#include "osmium/osm/way.hpp"
#include "osm2rdf/osm/Generic.h"
#include "util/geo/Geo.h"

namespace osm2rdf::osm {

class Way {
 public:
  typedef uint32_t id_t;
  Way();
  void finalize();
  explicit Way(const osmium::Way& way);
  [[nodiscard]] id_t id() const noexcept;
  [[nodiscard]] osm2rdf::osm::generic::changeset_id_t changeset() const noexcept;
  [[nodiscard]] std::time_t timestamp() const noexcept;
  [[nodiscard]] std::string user() const noexcept;
  [[nodiscard]] id_t uid() const noexcept;
  [[nodiscard]] osm2rdf::osm::generic::version_t version() const noexcept;
  [[nodiscard]] bool visible() const noexcept;
  [[nodiscard]] bool closed() const noexcept;
  [[nodiscard]] bool isArea() const noexcept;
  [[nodiscard]] const ::util::geo::DLine geom() const noexcept;
  [[nodiscard]] const osmium::WayNodeList& nodes() const noexcept;
  [[nodiscard]] const osmium::TagList& tags() const noexcept;

 protected:
  const osmium::Way* _w = 0;
};

}  // namespace osm2rdf::osm

#endif  // OSM2RDF_OSM_WAY_H_
