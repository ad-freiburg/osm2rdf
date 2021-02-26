// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_CONFIG_CONSTANTS_H
#define OSM2TTL_CONFIG_CONSTANTS_H

#include <string>

namespace osm2ttl::config::constants {

const static inline std::string HEADER = "Config";
const static inline std::string SECTION_MARKER = "---";

const static inline std::string SECTION_IO =
    SECTION_MARKER + " I/O " + SECTION_MARKER;

const static inline std::string SECTION_FACTS =
    SECTION_MARKER + " Facts " + SECTION_MARKER;
const static inline std::string NO_FACTS = "Not dumping facts";
const static inline std::string NO_AREA_FACTS = "Ignoring area facts";
const static inline std::string NO_NODE_FACTS = "Ignoring node facts";
const static inline std::string NO_RELATION_FACTS = "Ignoring relation facts";
const static inline std::string NO_WAY_FACTS = "Ignoring way facts";

const static inline std::string ADD_AREA_ENVELOPE = "Adding area envelopes";
const static inline std::string ADD_WAY_ENVELOPE = "Adding way envelopes";
const static inline std::string ADD_WAY_METADATA = "Adding way metadata";
const static inline std::string ADD_WAY_NODE_ORDER = "Adding way node order";

const static inline std::string SECTION_CONTAINS =
    SECTION_MARKER + " Contains " + SECTION_MARKER;
;

const static inline std::string NO_GEOM_RELATIONS =
    "Not dumping geometric relations";
const static inline std::string NO_AREA_GEOM_RELATIONS =
    "Ignoring area geometric relations";
const static inline std::string NO_NODE_GEOM_RELATIONS =
    "Ignoring node geometric relations";
const static inline std::string NO_WAY_GEOM_RELATIONS =
    "Ignoring way geometric relations";

const static inline std::string ADD_INVERSE_RELATION_DIRECTION =
    "Adding ogc:contained_by and ogc:intersected_by";

const static inline std::string SECTION_MISCELLANEOUS =
    SECTION_MARKER + " Miscellaneous " + SECTION_MARKER;

const static inline std::string SECTION_OPENMP =
    SECTION_MARKER + " OpenMP " + SECTION_MARKER;

}  // namespace osm2ttl::config::constants

#endif  // OSM2TTL_CONFIG_CONSTANTS_H
