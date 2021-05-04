// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#ifndef OSM2TTL_CONFIG_CONSTANTS_H
#define OSM2TTL_CONFIG_CONSTANTS_H

#include <string>

namespace osm2ttl::config::constants {

const static inline std::string BZIP2_EXTENSION = ".bz2";
const static inline std::string STATS_EXTENSION = ".stats";

const static inline std::string HEADER = "Config";

const static inline std::string SECTION_MARKER = "---";
const static inline std::string SECTION_IO =
    SECTION_MARKER + " I/O " + SECTION_MARKER;
const static inline std::string SECTION_FACTS =
    SECTION_MARKER + " Facts " + SECTION_MARKER;
const static inline std::string SECTION_CONTAINS =
    SECTION_MARKER + " Contains " + SECTION_MARKER;
const static inline std::string SECTION_MISCELLANEOUS =
    SECTION_MARKER + " Miscellaneous " + SECTION_MARKER;
const static inline std::string SECTION_OPENMP =
    SECTION_MARKER + " OpenMP " + SECTION_MARKER;

const static inline std::string HELP_OPTION_SHORT = "h";
const static inline std::string HELP_OPTION_LONG = "help";
const static inline std::string HELP_OPTION_HELP = "Display help information, use multiple times to display more.";

const static inline std::string CACHE_INFO = "Cache:";
const static inline std::string CACHE_OPTION_SHORT = "t";
const static inline std::string CACHE_OPTION_LONG = "cache";
const static inline std::string CACHE_OPTION_HELP = "Path to cache directory";

const static inline std::string INPUT_INFO = "Input:";

const static inline std::string OUTPUT_INFO = "Output:";
const static inline std::string OUTPUT_OPTION_SHORT = "o";
const static inline std::string OUTPUT_OPTION_LONG = "output";
const static inline std::string OUTPUT_OPTION_HELP = "Output file";

const static inline std::string OUTPUT_FORMAT_INFO = "Output format:";
const static inline std::string OUTPUT_FORMAT_OPTION_SHORT = "";
const static inline std::string OUTPUT_FORMAT_OPTION_LONG = "output-format";
const static inline std::string OUTPUT_FORMAT_OPTION_HELP =
    "Output format, valid values: nt, ttl, qlever";

const static inline std::string OUTPUT_NO_COMPRESS_OPTION_SHORT = "";
const static inline std::string OUTPUT_NO_COMPRESS_OPTION_LONG =
    "output-no-compress";
const static inline std::string OUTPUT_NO_COMPRESS_OPTION_HELP =
    "Do not compress output";

const static inline std::string STORE_LOCATIONS_ON_DISK_INFO =
    "Storing locations osmium locations on disk";
const static inline std::string STORE_LOCATIONS_ON_DISK_SHORT = "";
const static inline std::string STORE_LOCATIONS_ON_DISK_LONG =
    "store-locations-on-disk";
const static inline std::string STORE_LOCATIONS_ON_DISK_HELP =
    "Store locations on disk - slower but reduced ram usage.";

const static inline std::string NO_FACTS_INFO = "Not dumping facts";
const static inline std::string NO_FACTS_OPTION_SHORT = "";
const static inline std::string NO_FACTS_OPTION_LONG = "no-facts";
const static inline std::string NO_FACTS_OPTION_HELP = "Do not dump facts";

const static inline std::string NO_GEOM_RELATIONS_INFO =
    "Not dumping geometric relations";
const static inline std::string NO_GEOM_RELATIONS_OPTION_SHORT = "";
const static inline std::string NO_GEOM_RELATIONS_OPTION_LONG =
    "no-geometric-relations";
const static inline std::string NO_GEOM_RELATIONS_OPTION_HELP =
    "Do not dump geometric relations";

const static inline std::string NO_AREA_OPTION_SHORT = "";
const static inline std::string NO_AREA_OPTION_LONG = "no-areas";
const static inline std::string NO_AREA_OPTION_HELP = "Ignore areas";
const static inline std::string NO_AREA_FACTS_INFO = "Ignoring area facts";
const static inline std::string NO_AREA_FACTS_OPTION_SHORT = "";
const static inline std::string NO_AREA_FACTS_OPTION_LONG = "no-area-facts";
const static inline std::string NO_AREA_FACTS_OPTION_HELP =
    "Do not dump area facts";
const static inline std::string NO_AREA_GEOM_RELATIONS_INFO =
    "Ignoring area geometric relations";
const static inline std::string NO_AREA_GEOM_RELATIONS_OPTION_SHORT = "";
const static inline std::string NO_AREA_GEOM_RELATIONS_OPTION_LONG =
    "no-area-geometric-relations";
const static inline std::string NO_AREA_GEOM_RELATIONS_OPTION_HELP =
    "Do not dump area geometric relations";

const static inline std::string NO_NODE_OPTION_SHORT = "";
const static inline std::string NO_NODE_OPTION_LONG = "no-nodes";
const static inline std::string NO_NODE_OPTION_HELP = "Ignore nodes";
const static inline std::string NO_NODE_FACTS_INFO = "Ignoring node facts";
const static inline std::string NO_NODE_FACTS_OPTION_SHORT = "";
const static inline std::string NO_NODE_FACTS_OPTION_LONG = "no-node-facts";
const static inline std::string NO_NODE_FACTS_OPTION_HELP =
    "Do not dump node facts";
const static inline std::string NO_NODE_GEOM_RELATIONS_INFO =
    "Ignoring node geometric relations";
const static inline std::string NO_NODE_GEOM_RELATIONS_OPTION_SHORT = "";
const static inline std::string NO_NODE_GEOM_RELATIONS_OPTION_LONG =
    "no-node-geometric-relations";
const static inline std::string NO_NODE_GEOM_RELATIONS_OPTION_HELP =
    "Do not dump node geometric relations";

const static inline std::string NO_RELATION_OPTION_SHORT = "";
const static inline std::string NO_RELATION_OPTION_LONG = "no-relations";
const static inline std::string NO_RELATION_OPTION_HELP = "Ignore relations";
const static inline std::string NO_RELATION_FACTS_INFO =
    "Ignoring relation facts";
const static inline std::string NO_RELATION_FACTS_OPTION_SHORT = "";
const static inline std::string NO_RELATION_FACTS_OPTION_LONG =
    "no-relation-facts";
const static inline std::string NO_RELATION_FACTS_OPTION_HELP =
    "Do not dump relation facts";

const static inline std::string NO_WAY_OPTION_SHORT = "";
const static inline std::string NO_WAY_OPTION_LONG = "no-ways";
const static inline std::string NO_WAY_OPTION_HELP = "Ignore ways";
const static inline std::string NO_WAY_FACTS_INFO = "Ignoring way facts";
const static inline std::string NO_WAY_FACTS_OPTION_SHORT = "";
const static inline std::string NO_WAY_FACTS_OPTION_LONG = "no-way-facts";
const static inline std::string NO_WAY_FACTS_OPTION_HELP =
    "Do not dump way facts";
const static inline std::string NO_WAY_GEOM_RELATIONS_INFO =
    "Ignoring way geometric relations";
const static inline std::string NO_WAY_GEOM_RELATIONS_OPTION_SHORT = "";
const static inline std::string NO_WAY_GEOM_RELATIONS_OPTION_LONG =
    "no-way-geometric-relations";
const static inline std::string NO_WAY_GEOM_RELATIONS_OPTION_HELP =
    "Do not dump way geometric relations";

const static inline std::string ADD_AREA_ENVELOPE_INFO =
    "Adding area envelopes";
const static inline std::string ADD_AREA_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_AREA_ENVELOPE_OPTION_LONG =
    "add-area-envelope";
const static inline std::string ADD_AREA_ENVELOPE_OPTION_HELP =
    "Add envelope to areas.";

const static inline std::string ADD_AREA_ENVELOPE_RATIO_INFO =
    "Adding area/envelope ratios";
const static inline std::string ADD_AREA_ENVELOPE_RATIO_OPTION_SHORT = "";
const static inline std::string ADD_AREA_ENVELOPE_RATIO_OPTION_LONG =
    "add-area-envelope-ratio";
const static inline std::string ADD_AREA_ENVELOPE_RATIO_OPTION_HELP =
    "Add area/envelope ratio to areas.";

const static inline std::string ADD_WAY_ENVELOPE_INFO = "Adding way envelopes";
const static inline std::string ADD_WAY_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_WAY_ENVELOPE_OPTION_LONG =
    "add-way-envelope";
const static inline std::string ADD_WAY_ENVELOPE_OPTION_HELP =
    "Add envelope to ways.";

const static inline std::string ADD_WAY_METADATA_INFO = "Adding way metadata";
const static inline std::string ADD_WAY_METADATA_OPTION_SHORT = "";
const static inline std::string ADD_WAY_METADATA_OPTION_LONG =
    "add-way-metadata";
const static inline std::string ADD_WAY_METADATA_OPTION_HELP =
    "Add information about the way structure.";

const static inline std::string ADD_WAY_NODE_ORDER_INFO =
    "Adding way node order";
const static inline std::string ADD_WAY_NODE_ORDER_OPTION_SHORT = "";
const static inline std::string ADD_WAY_NODE_ORDER_OPTION_LONG =
    "add-way-node-order";
const static inline std::string ADD_WAY_NODE_ORDER_OPTION_HELP =
    "Add information about the node members in ways.";

const static inline std::string ADD_INVERSE_RELATION_DIRECTION_INFO =
    "Adding ogc:contained_by and ogc:intersected_by";
const static inline std::string ADD_INVERSE_RELATION_DIRECTION_SHORT = "";
const static inline std::string ADD_INVERSE_RELATION_DIRECTION_LONG =
    "add-inverse-relation-direction";
const static inline std::string ADD_INVERSE_RELATION_DIRECTION_HELP =
    "Store locations on disk - slower but reduced ram usage.";

const static inline std::string ADMIN_RELATIONS_ONLY_INFO =
    "Only handling nodes and relations with \"admin-level\" tag";
const static inline std::string ADMIN_RELATIONS_ONLY_OPTION_SHORT = "";
const static inline std::string ADMIN_RELATIONS_ONLY_OPTION_LONG =
    "admin-relations-only";
const static inline std::string ADMIN_RELATIONS_ONLY_OPTION_HELP =
    "Only handle nodes and relations with \"admin-level\" tag.";

const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_INFO =
    "Minimal area/envelope ratio: ";
const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_OPTION_SHORT = "";
const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_OPTION_LONG =
    "minimal-area-envelope-ratio";
const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_OPTION_HELP =
    "Minimal ratio of area to envelope to include a named area in the DAG."
    " Value <=0 includes every area.";

const static inline std::string SIMPLIFY_GEOMETRIES_INFO =
    "Simplifying relationship geometries with factor: ";
const static inline std::string SIMPLIFY_GEOMETRIES_OPTION_SHORT = "";
const static inline std::string SIMPLIFY_GEOMETRIES_OPTION_LONG =
    "simplify-geometries";
const static inline std::string SIMPLIFY_GEOMETRIES_OPTION_HELP =
    "Factor for geometry simplifaction, 0 to disable. This only affects "
    "relationship calculations and not the geometry dump.";

const static inline std::string SIMPLIFY_WKT_INFO = "Simplifying WKT";
const static inline std::string SIMPLIFY_WKT_OPTION_SHORT = "s";
const static inline std::string SIMPLIFY_WKT_OPTION_LONG = "simplify-wkt";
const static inline std::string SIMPLIFY_WKT_OPTION_HELP =
    "Simplify WKT-Geometries over this number of nodes, 0 to disable.";

const static inline std::string SIMPLIFY_WKT_DEVIATION_INFO =
    "Simplifying wkt geometries with factor: ";
const static inline std::string SIMPLIFY_WKT_DEVIATION_OPTION_SHORT = "";
const static inline std::string SIMPLIFY_WKT_DEVIATION_OPTION_LONG =
    "simplify-wkt-deviation";
const static inline std::string SIMPLIFY_WKT_DEVIATION_OPTION_HELP =
    "Same effect as simplify-geometries but for the simplification of dumped "
    "geometries.";

const static inline std::string SKIP_WIKI_LINKS_INFO =
    "Not adding wikipedia and wikidata links.";
const static inline std::string SKIP_WIKI_LINKS_OPTION_SHORT = "";
const static inline std::string SKIP_WIKI_LINKS_OPTION_LONG = "skip-wiki-links";
const static inline std::string SKIP_WIKI_LINKS_OPTION_HELP =
    "Do not add wikipedia and wikidata links.";

const static inline std::string SEMICOLON_TAG_KEYS_INFO =
    "Tag-Keys split by semicolon: ";
const static inline std::string SEMICOLON_TAG_KEYS_OPTION_SHORT = "";
const static inline std::string SEMICOLON_TAG_KEYS_OPTION_LONG =
    "split-tag-key-by-semicolon";
const static inline std::string SEMICOLON_TAG_KEYS_OPTION_HELP = "";

const static inline std::string OSM2TTL_PREFIX_INFO = "Prefix for osm2ttl-iris";
const static inline std::string OSM2TTL_PREFIX_OPTION_SHORT = "";
const static inline std::string OSM2TTL_PREFIX_OPTION_LONG = "oms2ttl-prefix";
const static inline std::string OSM2TTL_PREFIX_OPTION_HELP =
    "Prefix for own IRIs.";

const static inline std::string WKT_PRECISION_INFO =
    "Dumping WKT with precision: ";
const static inline std::string WKT_PRECISION_OPTION_SHORT = "";
const static inline std::string WKT_PRECISION_OPTION_LONG = "wkt-precision";
const static inline std::string WKT_PRECISION_OPTION_HELP =
    "Precision (number of decimal digits) for WKT coordinates.";

const static inline std::string WRITE_DAG_DOT_FILES_INFO =
    "Storing DAG states as .dot files";
const static inline std::string WRITE_DAG_DOT_FILES_OPTION_SHORT = "";
const static inline std::string WRITE_DAG_DOT_FILES_OPTION_LONG =
    "write-dag-dot-files";
const static inline std::string WRITE_DAG_DOT_FILES_OPTION_HELP =
    "Writes .dot files for DAG states.";

const static inline std::string WRITE_GEOM_RELATION_STATISTICS_INFO =
    "Storing statistics about geometry calculations - SLOW!";
const static inline std::string WRITE_GEOM_RELATION_STATISTICS_INFO_DISABLED =
    "Storing statistics not available - recompile with "
    "-DENABLE_GEOMETRY_STATISTIC";
const static inline std::string WRITE_GEOM_RELATION_STATISTICS_OPTION_SHORT =
    "";
const static inline std::string WRITE_GEOM_RELATION_STATISTICS_OPTION_LONG =
    "write-geometric-relation-statistics";
const static inline std::string WRITE_GEOM_RELATION_STATISTICS_OPTION_HELP =
    "Writes statistics about geometry calculations - slow!";

}  // namespace osm2ttl::config::constants

#endif  // OSM2TTL_CONFIG_CONSTANTS_H
