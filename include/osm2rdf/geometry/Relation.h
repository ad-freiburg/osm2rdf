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

#ifndef OSM2RDF_GEOMETRY_GEOMETRYCOLLECTION_H
#define OSM2RDF_GEOMETRY_GEOMETRYCOLLECTION_H

#include "boost/geometry/geometries/geometries.hpp"
#include "osm2rdf/geometry/Area.h"
#include "osm2rdf/geometry/Node.h"
#include "osm2rdf/geometry/Way.h"

namespace osm2rdf::geometry {
typedef boost::variant<osm2rdf::geometry::Node, osm2rdf::geometry::Way,
                       osm2rdf::geometry::Area>
    RelationGeometryParts_t;
typedef boost::geometry::model::geometry_collection<RelationGeometryParts_t>
    Relation;
}  // namespace osm2rdf::geometry

#endif  // OSM2RDF_GEOMETRY_GEOMETRYCOLLECTION_H
