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

#ifndef OSM2RDF_TTL_CONSTANTS_H
#define OSM2RDF_TTL_CONSTANTS_H

#include <string>

namespace osm2rdf::ttl::constants {

// Real constants
const static inline std::string NAMESPACE__GEOSPARQL = "geo";
const static inline std::string NAMESPACE__OHM_NODE = "ohmnode";
const static inline std::string NAMESPACE__OHM_RELATION = "ohmrel";
const static inline std::string NAMESPACE__OHM_WAY = "ohmway";
const static inline std::string NAMESPACE__OHM = "ohm";
const static inline std::string NAMESPACE__OPENGIS = "ogc";
const static inline std::string NAMESPACE__OSM_NODE = "osmnode";
const static inline std::string NAMESPACE__OSM_RELATION = "osmrel";
const static inline std::string NAMESPACE__OSM_TAG = "osmkey";
const static inline std::string NAMESPACE__OSM_WAY = "osmway";
const static inline std::string NAMESPACE__OSM_META = "osmmeta";
const static inline std::string NAMESPACE__OSM = "osm";
const static inline std::string NAMESPACE__OSM2RDF = "osm2rdf";
const static inline std::string NAMESPACE__OSM2RDF_GEOM = "osm2rdfgeom";
const static inline std::string NAMESPACE__OSM2RDF_MEMBER = "osm2rdfmember";
const static inline std::string NAMESPACE__OSM2RDF_META = "osm2rdfmeta";
const static inline std::string NAMESPACE__OSM2RDF_TAG = "osm2rdfkey";
const static inline std::string NAMESPACE__RDF = "rdf";
const static inline std::string NAMESPACE__WIKIDATA_ENTITY = "wd";
const static inline std::string NAMESPACE__XML_SCHEMA = "xsd";

// Generated constants (depending on output format)
inline std::string IRI__GEOSPARQL__AS_WKT;
inline std::string IRI__GEOSPARQL__HAS_CENTROID;
inline std::string IRI__GEOSPARQL__HAS_GEOMETRY;
inline std::string IRI__GEOSPARQL__HAS_SERIALIZATION;
inline std::string IRI__GEOSPARQL__WKT_LITERAL;

inline std::string IRI__OPENGIS_CONTAINS;
inline std::string IRI__OSM2RDF_CONTAINS_NON_AREA;
inline std::string IRI__OSM2RDF_CONTAINS_AREA;
inline std::string IRI__OPENGIS_INTERSECTS;
inline std::string IRI__OSM2RDF_INTERSECTS_NON_AREA;
inline std::string IRI__OSM2RDF_INTERSECTS_AREA;
inline std::string IRI__OPENGIS_COVERS;
inline std::string IRI__OPENGIS_TOUCHES;
inline std::string IRI__OPENGIS_EQUALS;
inline std::string IRI__OPENGIS_CROSSES;
inline std::string IRI__OPENGIS_OVERLAPS;

inline std::string IRI__OSM2RDF_GEOM__CONVEX_HULL;
inline std::string IRI__OSM2RDF_GEOM__ENVELOPE;
inline std::string IRI__OSM2RDF_GEOM__OBB;
inline std::string IRI__OSM2RDF_MEMBER__ID;
inline std::string IRI__OSM2RDF_MEMBER__ROLE;
inline std::string IRI__OSM2RDF_MEMBER__POS;
inline std::string IRI__OSM2RDF__LENGTH;
inline std::string IRI__OSMMETA_TIMESTAMP;
inline std::string IRI__OSMWAY_IS_CLOSED;
inline std::string IRI__OSMWAY_NEXT_NODE;
inline std::string IRI__OSMWAY_NEXT_NODE_DISTANCE;
inline std::string IRI__OSMWAY_NODE;
inline std::string IRI__OSMWAY_NODE_COUNT;
inline std::string IRI__OSMWAY_UNIQUE_NODE_COUNT;
inline std::string IRI__OSM_NODE;
inline std::string IRI__OSM_RELATION;
inline std::string IRI__OSM_TAG;
inline std::string IRI__OSM_WAY;

inline std::string IRI__RDF_TYPE;

inline std::string IRI__XSD_DATE;
inline std::string IRI__XSD_DATE_TIME;
inline std::string IRI__XSD_DECIMAL;
inline std::string IRI__XSD_DOUBLE;
inline std::string IRI__XSD_FLOAT;
inline std::string IRI__XSD_INTEGER;
inline std::string IRI__XSD_YEAR;
inline std::string IRI__XSD_YEAR_MONTH;

inline std::string LITERAL__NO;
inline std::string LITERAL__YES;

// Arrays holding values depending on the used dataset
const static inline std::vector<std::string> DATASET_ID = {"osm", "ohm"};
const static inline std::vector<std::string> NODE_NAMESPACE = {
    NAMESPACE__OSM_NODE, NAMESPACE__OHM_NODE};
const static inline std::vector<std::string> RELATION_NAMESPACE = {
    NAMESPACE__OSM_RELATION, NAMESPACE__OHM_RELATION};
const static inline std::vector<std::string> WAY_NAMESPACE = {
    NAMESPACE__OSM_WAY, NAMESPACE__OHM_WAY};

}  // namespace osm2rdf::ttl::constants

#endif  // OSM2RDF_TTL_CONSTANTS_H
