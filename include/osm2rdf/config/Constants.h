// Copyright 2020 - 2022, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>
//          Hannah Bast <bast@cs.uni-freiburg.de>

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

#ifndef OSM2RDF_CONFIG_CONSTANTS_H
#define OSM2RDF_CONFIG_CONSTANTS_H

#include <string>

namespace osm2rdf::config::constants {

const static inline std::string BZIP2_EXTENSION = ".bz2";
const static inline std::string STATS_EXTENSION = ".stats";
const static inline std::string CONTAINS_STATS_EXTENSION = ".contains-stats";
const static inline std::string JSON_EXTENSION = ".json";

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
const static inline std::string HELP_OPTION_HELP =
    "Display help information, use multiple times to display more";

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

const static inline std::string OUTPUT_KEEP_FILES_OPTION_SHORT = "";
const static inline std::string OUTPUT_KEEP_FILES_OPTION_LONG =
    "output-keep-files";
const static inline std::string OUTPUT_KEEP_FILES_OPTION_HELP =
    "Do not remove temporary output files";
const static inline std::string OUTPUT_KEEP_FILES_OPTION_INFO =
    "Keeping temporary output files";

const static inline std::string OUTPUT_NO_COMPRESS_OPTION_SHORT = "";
const static inline std::string OUTPUT_NO_COMPRESS_OPTION_LONG =
    "output-no-compress";
const static inline std::string OUTPUT_NO_COMPRESS_OPTION_HELP =
    "Do not compress output";

const static inline std::string STORE_LOCATIONS_ON_DISK_INFO =
    "Storing locations osmium locations on disk:";
const static inline std::string STORE_LOCATIONS_ON_DISK_SHORT = "";
const static inline std::string STORE_LOCATIONS_ON_DISK_LONG =
    "store-locations-on-disk";
const static inline std::string STORE_LOCATIONS_ON_DISK_HELP =
    "Store locations on disk, optional valid values: sparse (default), dense";

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

const static inline std::string GEOM_REL_TRANS_REDUCE_INFO =
    "Only writing transitive reduction of contains relation";
const static inline std::string GEOM_REL_TRANS_REDUCE_OPTION_SHORT = "";
const static inline std::string GEOM_REL_TRANS_REDUCE_OPTION_LONG =
    "transitive-reduction"
const static inline std::string GEOM_REL_TRANS_REDUCE_OPTION_HELP =
    "Only write transitive reduction of contains relation";

const static inline std::string WRITE_SPLIT_GEOM_REL_INFO =
    "Writing deprecated contains and intersects relations with suffix _area and _nonarea";
const static inline std::string WRITE_SPLIT_GEOM_REL_OPTION_SHORT = "";
const static inline std::string WRITE_SPLIT_GEOM_REL_OPTION_LONG =
    "write-osm2rdf-contains-and-intersects-triples";
const static inline std::string WRITE_SPLIT_GEOM_REL_OPTION_HELP =
    "Write deprecated contains and intersects relations with suffix _area and _nonarea";

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

const static inline std::string ADD_AREA_CONVEX_HULL_INFO =
    "Adding area convex hulls";
const static inline std::string ADD_AREA_CONVEX_HULL_OPTION_SHORT = "";
const static inline std::string ADD_AREA_CONVEX_HULL_OPTION_LONG =
    "add-area-convex-hull";
const static inline std::string ADD_AREA_CONVEX_HULL_OPTION_HELP =
    "Add convex hull to areas";

const static inline std::string ADD_AREA_ENVELOPE_INFO =
    "Adding area envelopes";
const static inline std::string ADD_AREA_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_AREA_ENVELOPE_OPTION_LONG =
    "add-area-envelope";
const static inline std::string ADD_AREA_ENVELOPE_OPTION_HELP =
    "Add envelope to areas";

const static inline std::string ADD_AREA_ENVELOPE_RATIO_INFO =
    "Adding area/envelope ratios";
const static inline std::string ADD_AREA_ENVELOPE_RATIO_OPTION_SHORT = "";
const static inline std::string ADD_AREA_ENVELOPE_RATIO_OPTION_LONG =
    "add-area-envelope-ratio";
const static inline std::string ADD_AREA_ENVELOPE_RATIO_OPTION_HELP =
    "Add area/envelope ratio to areas";

const static inline std::string ADD_AREA_ORIENTED_BOUNDING_BOX_INFO =
    "Adding area oriented-bounding-boxes";
const static inline std::string ADD_AREA_ORIENTED_BOUNDING_BOX_OPTION_SHORT = "";
const static inline std::string ADD_AREA_ORIENTED_BOUNDING_BOX_OPTION_LONG =
    "add-area-oriented-bounding-box";
const static inline std::string ADD_AREA_ORIENTED_BOUNDING_BOX_OPTION_HELP =
    "Add oriented-bounding-box to areas";

const static inline std::string HASGEOMETRY_AS_WKT_INFO =
    "Letting geo:hasGeometry point directly to WKT serialization literal";
const static inline std::string HASGEOMETRY_AS_WKT_OPTION_SHORT = "";
const static inline std::string HASGEOMETRY_AS_WKT_OPTION_LONG =
    "hasgeometry-as-wkt";
const static inline std::string HASGEOMETRY_AS_WKT_OPTION_HELP =
    "Let geo:hasGeometry point directly to a WKT literal";

const static inline std::string ADD_NODE_CONVEX_HULL_INFO =
    "Adding node convex hulls";
const static inline std::string ADD_NODE_CONVEX_HULL_OPTION_SHORT = "";
const static inline std::string ADD_NODE_CONVEX_HULL_OPTION_LONG =
    "add-node-convex-hull";
const static inline std::string ADD_NODE_CONVEX_HULL_OPTION_HELP =
    "Add convex hull to nodes";

const static inline std::string ADD_NODE_ENVELOPE_INFO =
    "Adding node envelopes";
const static inline std::string ADD_NODE_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_NODE_ENVELOPE_OPTION_LONG =
    "add-node-envelope";
const static inline std::string ADD_NODE_ENVELOPE_OPTION_HELP =
    "Add envelope to nodes";

const static inline std::string ADD_NODE_ORIENTED_BOUNDING_BOX_INFO =
    "Adding node oriented-bounding-boxes";
const static inline std::string ADD_NODE_ORIENTED_BOUNDING_BOX_OPTION_SHORT = "";
const static inline std::string ADD_NODE_ORIENTED_BOUNDING_BOX_OPTION_LONG =
    "add-node-oriented-bounding-box";
const static inline std::string ADD_NODE_ORIENTED_BOUNDING_BOX_OPTION_HELP =
    "Add oriented-bounding-box to nodes";

const static inline std::string ADD_RELATION_BORDER_MEMBERS_INFO =
    "Adding relation border members";
const static inline std::string ADD_RELATION_BORDER_MEMBERS_OPTION_SHORT = "";
const static inline std::string ADD_RELATION_BORDER_MEMBERS_OPTION_LONG =
    "add-relation-border-members";
const static inline std::string ADD_RELATION_BORDER_MEMBERS_OPTION_HELP =
    "Add relation border members (inner and outer)";

const static inline std::string ADD_RELATION_CONVEX_HULL_INFO =
    "Adding relation convex hulls";
const static inline std::string ADD_RELATION_CONVEX_HULL_OPTION_SHORT = "";
const static inline std::string ADD_RELATION_CONVEX_HULL_OPTION_LONG =
    "add-relation-convex-hull";
const static inline std::string ADD_RELATION_CONVEX_HULL_OPTION_HELP =
    "Add convex hull to relations";

const static inline std::string ADD_RELATION_ENVELOPE_INFO = "Adding relation envelopes";
const static inline std::string ADD_RELATION_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_RELATION_ENVELOPE_OPTION_LONG =
    "add-relation-envelope";
const static inline std::string ADD_RELATION_ENVELOPE_OPTION_HELP =
    "Add envelope to relations";

const static inline std::string ADD_RELATION_ORIENTED_BOUNDING_BOX_INFO =
    "Adding relation oriented-bounding-boxes";
const static inline std::string ADD_RELATION_ORIENTED_BOUNDING_BOX_OPTION_SHORT = "";
const static inline std::string ADD_RELATION_ORIENTED_BOUNDING_BOX_OPTION_LONG =
    "add-relation-oriented-bounding-box";
const static inline std::string ADD_RELATION_ORIENTED_BOUNDING_BOX_OPTION_HELP =
    "Add oriented-bounding-box to relations";

const static inline std::string ADD_WAY_CONVEX_HULL_INFO =
    "Adding way convex hulls";
const static inline std::string ADD_WAY_CONVEX_HULL_OPTION_SHORT = "";
const static inline std::string ADD_WAY_CONVEX_HULL_OPTION_LONG =
    "add-way-convex-hull";
const static inline std::string ADD_WAY_CONVEX_HULL_OPTION_HELP =
    "Add convex hull to ways";

const static inline std::string ADD_WAY_ENVELOPE_INFO = "Adding way envelopes";
const static inline std::string ADD_WAY_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_WAY_ENVELOPE_OPTION_LONG =
    "add-way-envelope";
const static inline std::string ADD_WAY_ENVELOPE_OPTION_HELP =
    "Add envelope to ways";

const static inline std::string ADD_WAY_ORIENTED_BOUNDING_BOX_INFO =
    "Adding way oriented-bounding-boxes";
const static inline std::string ADD_WAY_ORIENTED_BOUNDING_BOX_OPTION_SHORT = "";
const static inline std::string ADD_WAY_ORIENTED_BOUNDING_BOX_OPTION_LONG =
    "add-way-oriented-bounding-box";
const static inline std::string ADD_WAY_ORIENTED_BOUNDING_BOX_OPTION_HELP =
    "Add oriented-bounding-box to ways";

const static inline std::string ADD_WAY_METADATA_INFO = "Adding way metadata";
const static inline std::string ADD_WAY_METADATA_OPTION_SHORT = "";
const static inline std::string ADD_WAY_METADATA_OPTION_LONG =
    "add-way-metadata";
const static inline std::string ADD_WAY_METADATA_OPTION_HELP =
    "Add information about the way structure";

const static inline std::string ADD_WAY_NODE_GEOMETRY_INFO =
    "Adding way member geometries";
const static inline std::string ADD_WAY_NODE_GEOMETRY_OPTION_SHORT = "";
const static inline std::string ADD_WAY_NODE_GEOMETRY_OPTION_LONG =
    "add-way-node-geometry";
const static inline std::string ADD_WAY_NODE_GEOMETRY_OPTION_HELP =
    "Add explicit geometry for every node member";

const static inline std::string ADD_WAY_NODE_ORDER_INFO =
    "Adding way node order";
const static inline std::string ADD_WAY_NODE_ORDER_OPTION_SHORT = "";
const static inline std::string ADD_WAY_NODE_ORDER_OPTION_LONG =
    "add-way-node-order";
const static inline std::string ADD_WAY_NODE_ORDER_OPTION_HELP =
    "Add information about the node members in ways";

const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_INFO =
    "Adding way metadata";
const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_OPTION_SHORT = "";
const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_OPTION_LONG =
    "add-way-node-spatial-metadata";
const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_OPTION_HELP =
    "Add spatial information about the relations of member nodes in ways";

const static inline std::string ADMIN_RELATIONS_ONLY_INFO =
    "Only handling nodes and relations with \"admin-level\" tag";
const static inline std::string ADMIN_RELATIONS_ONLY_OPTION_SHORT = "";
const static inline std::string ADMIN_RELATIONS_ONLY_OPTION_LONG =
    "admin-relations-only";
const static inline std::string ADMIN_RELATIONS_ONLY_OPTION_HELP =
    "Only handle nodes and relations with \"admin-level\" tag";

const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_INFO =
    "Minimal area/envelope ratio: ";
const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_OPTION_SHORT = "";
const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_OPTION_LONG =
    "minimal-area-envelope-ratio";
const static inline std::string MINIMAL_AREA_ENVELOPE_RATIO_OPTION_HELP =
    "Minimal ratio of area to envelope to include a named area in the DAG;"
    " Value <=0 includes every area";

const static inline std::string SIMPLIFY_GEOMETRIES_INFO =
    "Simplifying relationship geometries with factor: ";
const static inline std::string SIMPLIFY_GEOMETRIES_OPTION_SHORT = "";
const static inline std::string SIMPLIFY_GEOMETRIES_OPTION_LONG =
    "simplify-geometries";
const static inline std::string SIMPLIFY_GEOMETRIES_OPTION_HELP =
    "Factor for geometry simplifaction, 0 to disable; will be multiplied with "
    "the geometry "
    "perimeter or length. This only affects "
    "relationship calculations and not the geometry dump";

const static inline std::string SIMPLIFY_GEOMETRIES_INNER_OUTER_INFO =
    "Simplifying inner/outer geometries with factor: ";
const static inline std::string SIMPLIFY_GEOMETRIES_INNER_OUTER_OPTION_SHORT =
    "";
const static inline std::string SIMPLIFY_GEOMETRIES_INNER_OUTER_OPTION_LONG =
    "simplify-inner-outer-geometries";
const static inline std::string SIMPLIFY_GEOMETRIES_INNER_OUTER_OPTION_HELP =
    "Factor for geometry simplifaction of inner/outer geometries, will be "
    "multiplied with the geometry perimter. This only affects "
    "relationship calculations and not the geometry dump";

const static inline std::string DONT_USE_INNER_OUTER_GEOMETRIES_INFO =
    "Don't use inner/outer simplified geometries of areas: ";
const static inline std::string DONT_USE_INNER_OUTER_GEOMETRIES_OPTION_SHORT =
    "";
const static inline std::string DONT_USE_INNER_OUTER_GEOMETRIES_OPTION_LONG =
    "no-inner-outer";
const static inline std::string DONT_USE_INNER_OUTER_GEOMETRIES_OPTION_HELP =
    "Don't use inner/outer simplified geometries of areas for contains "
    "relation.";

const static inline std::string APPROX_SPATIAL_REL_INFO =
    "Approximate spatial relations using inner/outer simplified geometries.";
const static inline std::string APPROX_SPATIAL_REL_OPTION_SHORT =
    "";
const static inline std::string APPROX_SPATIAL_REL_OPTION_LONG =
    "approximate-spatial-relations";
const static inline std::string APPROX_SPATIAL_REL_OPTION_HELP = "Use "
  "simplified inner/outer geometries for approximate calcuation of spatial "
  "relations";

const static inline std::string SIMPLIFY_WKT_INFO = "Simplifying WKT";
const static inline std::string SIMPLIFY_WKT_OPTION_SHORT = "s";
const static inline std::string SIMPLIFY_WKT_OPTION_LONG = "simplify-wkt";
const static inline std::string SIMPLIFY_WKT_OPTION_HELP =
    "Simplify WKT-Geometries over this number of nodes, 0 to disable";

const static inline std::string SIMPLIFY_WKT_DEVIATION_INFO =
    "Simplifying wkt geometries with factor: ";
const static inline std::string SIMPLIFY_WKT_DEVIATION_OPTION_SHORT = "";
const static inline std::string SIMPLIFY_WKT_DEVIATION_OPTION_LONG =
    "simplify-wkt-deviation";
const static inline std::string SIMPLIFY_WKT_DEVIATION_OPTION_HELP =
    "Same effect as simplify-geometries but for the simplification of dumped "
    "geometries";

const static inline std::string SKIP_WIKI_LINKS_INFO =
    "Not adding wikipedia and wikidata links.";
const static inline std::string SKIP_WIKI_LINKS_OPTION_SHORT = "";
const static inline std::string SKIP_WIKI_LINKS_OPTION_LONG = "skip-wiki-links";
const static inline std::string SKIP_WIKI_LINKS_OPTION_HELP =
    "Do not add wikipedia and wikidata links";

const static inline std::string SEMICOLON_TAG_KEYS_INFO =
    "Tag-Keys split by semicolon: ";
const static inline std::string SEMICOLON_TAG_KEYS_OPTION_SHORT = "";
const static inline std::string SEMICOLON_TAG_KEYS_OPTION_LONG =
    "split-tag-key-by-semicolon";
const static inline std::string SEMICOLON_TAG_KEYS_OPTION_HELP = "";

const static inline std::string WKT_PRECISION_INFO =
    "Dumping WKT with precision: ";
const static inline std::string WKT_PRECISION_OPTION_SHORT = "";
const static inline std::string WKT_PRECISION_OPTION_LONG = "wkt-precision";
const static inline std::string WKT_PRECISION_OPTION_HELP =
    "Precision (number of decimal digits) for WKT coordinates";

const static inline std::string WRITE_DAG_DOT_FILES_INFO =
    "Storing DAG states as .dot files";
const static inline std::string WRITE_DAG_DOT_FILES_OPTION_SHORT = "";
const static inline std::string WRITE_DAG_DOT_FILES_OPTION_LONG =
    "write-dag-dot-files";
const static inline std::string WRITE_DAG_DOT_FILES_OPTION_HELP =
    "Writes .dot files for DAG states";

const static inline std::string WRITE_RDF_STATISTICS_INFO =
    "Storing RDF statistics as .stats.json";
const static inline std::string WRITE_RDF_STATISTICS_OPTION_SHORT = "";
const static inline std::string WRITE_RDF_STATISTICS_OPTION_LONG =
    "write-rdf-statistics";
const static inline std::string WRITE_RDF_STATISTICS_OPTION_HELP =
    "Storing RDF statistics as .stats.json";

}  // namespace osm2rdf::config::constants

#endif  // OSM2RDF_CONFIG_CONSTANTS_H
