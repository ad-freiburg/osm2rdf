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

#ifndef OSM2RDF_GEOMETRY_GLOBAL_H
#define OSM2RDF_GEOMETRY_GLOBAL_H

namespace osm2rdf::geometry {
// Location type used by all geometry classes.
typedef double location_coordinate_t;

// Area type used to represent the area of areas.
typedef float area_result_t;
}  // namespace osm2rdf::geometry

#endif  // OSM2RDF_GEOMETRY_GLOBAL_H
