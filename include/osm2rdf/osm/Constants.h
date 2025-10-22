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

#ifndef OSM2RDF_OSM_CONSTANTS_H
#define OSM2RDF_OSM_CONSTANTS_H

namespace osm2rdf::osm::constants {

static const int AREA_PRECISION = 4;
static const int LENGTH_PRECISION = 2;
static const int BASE10_BASE = 10;
static const double BASE_SIMPLIFICATION_FACTOR = 0.001;

static const double INNER_OUTER_SIMPLIFICATION_FACTOR = 0.01;

static const double DEGREE = 3.141592 / 180.0;
static const double EARTH_RADIUS_KM = 6371.0;
static const int METERS_IN_KM = 1000;
}  // namespace osm2rdf::osm::constants

#endif  // OSM2RDF_OSM_CONSTANTS_H
