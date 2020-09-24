// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_CONSTANTS_H
#define OSM2TTL_TTL_CONSTANTS_H

#include <string>

namespace osm2ttl {
namespace ttl {
namespace constants {

// Real constants
const static inline std::string NAMESPACE__GEOSPARQL = "geo";
const static inline std::string NAMESPACE__OPENGIS = "ogc";
const static inline std::string NAMESPACE__OSM_META = "osmm";
const static inline std::string NAMESPACE__OSM_NODE = "osmnode";
const static inline std::string NAMESPACE__OSM_RELATION = "osmrel";
const static inline std::string NAMESPACE__OSM_TAG = "osmt";
const static inline std::string NAMESPACE__OSM_WAY = "osmway";
const static inline std::string NAMESPACE__OSM = "osm";
const static inline std::string NAMESPACE__RDF = "rdf";
const static inline std::string NAMESPACE__WIKIDATA_ENTITY = "wd";
const static inline std::string NAMESPACE__XML_SCHEMA = "xsd";

// Generated constants (depending on output format)
inline std::string IRI__GEOSPARQL__HAS_GEOMETRY;
inline std::string IRI__GEOSPARQL__WKT_LITERAL;
inline std::string IRI_OGC_CONTAINS;
inline std::string IRI__OSM_META__POS;
inline std::string IRI_OSMWAY_ISCLOSED;
inline std::string IRI_OSMWAY_NODE;
inline std::string IRI_OSMWAY_NODECOUNT;
inline std::string IRI_OSMWAY_UNIQUENODECOUNT;
inline std::string IRI_OSM_ENVELOPE;
inline std::string IRI_OSM_NODE;
inline std::string IRI_OSM_RELATION;
inline std::string IRI_OSM_WAY;
inline std::string IRI_OSM_WIKIPEDIA;
inline std::string IRI_RDF_TYPE;
inline std::string IRI_XSD_INTEGER;

inline std::string LITERAL_NO;
inline std::string LITERAL_YES;

}  // namespace constants
}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_CONSTANTS_H
