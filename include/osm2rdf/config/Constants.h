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
const static inline std::string GZ_EXTENSION = ".gz";
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

const static inline std::string OUTPUT_COMPRESS_OPTION_SHORT = "";
const static inline std::string OUTPUT_COMPRESS_OPTION_LONG =
    "output-compression";
const static inline std::string OUTPUT_COMPRESS_OPTION_HELP =
    "Output file compression, valid values: none, bz2, gz";

const static inline std::string STORE_LOCATIONS_INFO =
    "Storing locations osmium locations:";
const static inline std::string STORE_LOCATIONS_SHORT = "";
const static inline std::string STORE_LOCATIONS_LONG = "store-locations";
const static inline std::string STORE_LOCATIONS_HELP =
    "Method used to store locations, valid values: mem-flex (default), "
    "mem-dense, disk-sparse, disk-dense ";

const static inline std::string NO_OSM_METADATA_INFO =
    "Not outputting OSM metadata";
const static inline std::string NO_OSM_METADATA_OPTION_SHORT = "";
const static inline std::string NO_OSM_METADATA_OPTION_LONG = "no-osm-metadata";
const static inline std::string NO_OSM_METADATA_OPTION_HELP =
    "Do not output OSM metadata (user, timestamp, changeset, visibility and "
    "version)";

const static inline std::string NO_FACTS_INFO = "Not dumping facts";
const static inline std::string NO_FACTS_OPTION_SHORT = "";
const static inline std::string NO_FACTS_OPTION_LONG = "no-facts";
const static inline std::string NO_FACTS_OPTION_HELP = "Do not dump facts";

const static inline std::string SOURCE_DATASET_INFO = "Source dataset";
const static inline std::string SOURCE_DATASET_OPTION_SHORT = "";
const static inline std::string SOURCE_DATASET_OPTION_LONG = "source-dataset";
const static inline std::string SOURCE_DATASET_OPTION_HELP =
    "Source dataset, either 'OSM', or 'OHM'";

const static inline std::string OGC_GEO_TRIPLES_INFO =
    "Writing mode of OGC-style geometric triples";
const static inline std::string OGC_GEO_TRIPLES_OPTION_SHORT = "";
const static inline std::string OGC_GEO_TRIPLES_OPTION_LONG =
    "write-ogc-geo-triples";
const static inline std::string OGC_GEO_TRIPLES_OPTION_HELP =
    "Writing of OGC-style geometric triples, either 'full', or 'none'";

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
const static inline std::string NO_RELATION_GEOM_RELATIONS_INFO =
    "Ignoring relation geometric relations";
const static inline std::string NO_RELATION_GEOM_RELATIONS_OPTION_SHORT = "";
const static inline std::string NO_RELATION_GEOM_RELATIONS_OPTION_LONG =
    "no-relation-geometric-relations";
const static inline std::string NO_RELATION_GEOM_RELATIONS_OPTION_HELP =
    "Do not dump relation geometric relations";

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

const static inline std::string ADD_ZERO_FACT_NUMBER_INFO =
  "Also output osm2rdf:fact triples with fact number 0";
const static inline std::string ADD_ZERO_FACT_NUMBER_OPTION_SHORT = "";
const static inline std::string ADD_ZERO_FACT_NUMBER_OPTION_LONG =
  "add-zero-fact-number";
const static inline std::string ADD_ZERO_FACT_NUMBER_OPTION_HELP =
  "Also output osm2rdf:fact triples with fact number 0, that is, "
  "for untagged nodes, ways, relations and areas";


const static inline std::string UNTAGGED_NODES_SPATIAL_RELS_INFO =
    "Compute spatial relations involving untagged nodes";
const static inline std::string UNTAGGED_NODES_SPATIAL_RELS_OPTION_SHORT = "";
const static inline std::string UNTAGGED_NODES_SPATIAL_RELS_OPTION_LONG =
    "add-untagged-nodes-geometric-relations";
const static inline std::string UNTAGGED_NODES_SPATIAL_RELS_OPTION_HELP =
    "Compute spatial relations involving untagged nodes";

const static inline std::string NO_UNTAGGED_NODES_INFO =
    "Do not output untagged nodes";
const static inline std::string NO_UNTAGGED_NODES_OPTION_SHORT = "";
const static inline std::string NO_UNTAGGED_NODES_OPTION_LONG =
    "no-untagged-nodes";
const static inline std::string NO_UNTAGGED_NODES_OPTION_HELP =
    "Do not output untagged nodes";

const static inline std::string NO_UNTAGGED_WAYS_INFO =
    "Do not output untagged ways";
const static inline std::string NO_UNTAGGED_WAYS_OPTION_SHORT = "";
const static inline std::string NO_UNTAGGED_WAYS_OPTION_LONG =
    "no-untagged-ways";
const static inline std::string NO_UNTAGGED_WAYS_OPTION_HELP =
    "Do not output untagged ways";

const static inline std::string NO_UNTAGGED_RELATIONS_INFO =
    "Do not output untagged relations";
const static inline std::string NO_UNTAGGED_RELATIONS_OPTION_SHORT = "";
const static inline std::string NO_UNTAGGED_RELATIONS_OPTION_LONG =
    "no-untagged-relations";
const static inline std::string NO_UNTAGGED_RELATIONS_OPTION_HELP =
    "Do not output untagged relations";

const static inline std::string NO_UNTAGGED_AREAS_INFO =
    "Do not output untagged areas";
const static inline std::string NO_UNTAGGED_AREAS_OPTION_SHORT = "";
const static inline std::string NO_UNTAGGED_AREAS_OPTION_LONG =
    "no-untagged-areas";
const static inline std::string NO_UNTAGGED_AREAS_OPTION_HELP =
    "Do not output untagged areas";

const static inline std::string ADD_AREA_WAY_LINESTRINGS_INFO =
    "Adding linestrings for ways which form areas";
const static inline std::string ADD_AREA_WAY_LINESTRINGS_OPTION_SHORT = "";
const static inline std::string ADD_AREA_WAY_LINESTRINGS_OPTION_LONG =
    "add-area-way-linestrings";
const static inline std::string ADD_AREA_WAY_LINESTRINGS_OPTION_HELP =
    "Add linestrings for ways which form areas";

const static inline std::string ADD_CENTROID_INFO =
    "Adding a geo:hasCentroid triple for each geometry";
const static inline std::string ADD_CENTROID_OPTION_SHORT = "";
const static inline std::string ADD_CENTROID_OPTION_LONG = "add-hascentroid";
const static inline std::string ADD_CENTROID_OPTION_HELP =
    "Add a geo:hasCentroid triple for each geometry";

const static inline std::string ADD_ENVELOPE_INFO =
  "Adding a geo:hasEnvelope triple for each geometry";
const static inline std::string ADD_ENVELOPE_OPTION_SHORT = "";
const static inline std::string ADD_ENVELOPE_OPTION_LONG = "add-hasenvelope";
const static inline std::string ADD_ENVELOPE_OPTION_HELP =
    "Add a geo:hasEnvelope triple for each geometry";

const static inline std::string ADD_OBB_INFO =
  "Adding a geo:hasObb triple for each geometry";
const static inline std::string ADD_OBB_OPTION_SHORT = "";
const static inline std::string ADD_OBB_OPTION_LONG = "add-hasobb";
const static inline std::string ADD_OBB_OPTION_HELP =
    "Add a geo:hasObb triple for each geometry";

const static inline std::string ADD_CONVEX_HULL_INFO =
  "Adding a geo:hasConvexHull triple for each geometry";
const static inline std::string ADD_CONVEX_HULL_OPTION_SHORT = "";
const static inline std::string ADD_CONVEX_HULL_OPTION_LONG =
    "add-hasconvexhull";
const static inline std::string ADD_CONVEX_HULL_OPTION_HELP =
  "Add a geo:hasConvexHull triple for each geometry";

const static inline std::string ADD_WAY_METADATA_INFO = "Adding way metadata";
const static inline std::string ADD_WAY_METADATA_OPTION_SHORT = "";
const static inline std::string ADD_WAY_METADATA_OPTION_LONG =
    "add-way-metadata";
const static inline std::string ADD_WAY_METADATA_OPTION_HELP =
    "Add information about the way structure";

const static inline std::string NO_MEMBER_TRIPLES_INFO =
    "Do not write member triples for relations and ways";
const static inline std::string NO_MEMBER_TRIPLES_OPTION_SHORT = "";
const static inline std::string NO_MEMBER_TRIPLES_OPTION_LONG =
    "no-member-triples";
const static inline std::string NO_MEMBER_TRIPLES_OPTION_HELP =
    "Do not write member triples for relations and ways";

const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_INFO =
    "Adding way metadata";
const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_OPTION_SHORT = "";
const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_OPTION_LONG =
    "add-way-node-spatial-metadata";
const static inline std::string ADD_WAY_NODE_SPATIAL_METADATA_OPTION_HELP =
    "Add spatial information about the relations of member nodes in ways";

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

const static inline std::string AUX_GEO_FILES_INFO =
    "Auxiliary geo files for computing spatial relations";
const static inline std::string AUX_GEO_FILES_OPTION_SHORT = "";
const static inline std::string AUX_GEO_FILES_OPTION_LONG = "aux-geo-files";
const static inline std::string AUX_GEO_FILES_OPTION_HELP =
    "Auxiliary geo files for computing spatial relations";

const static inline std::string NUM_THREADS_INFO = "Number of threads to use";
const static inline std::string NUM_THREADS_OPTION_SHORT = "";
const static inline std::string NUM_THREADS_OPTION_LONG = "num-threads";
const static inline std::string NUM_THREADS_OPTION_HELP =
    "Number of threads to use";

const static inline std::string WKT_PRECISION_INFO =
    "Dumping WKT with precision: ";
const static inline std::string WKT_PRECISION_OPTION_SHORT = "";
const static inline std::string WKT_PRECISION_OPTION_LONG = "wkt-precision";
const static inline std::string WKT_PRECISION_OPTION_HELP =
    "Precision (number of decimal digits) for WKT coordinates";

const static inline std::string WRITE_RDF_STATISTICS_INFO =
    "Storing RDF statistics as .stats.json";
const static inline std::string WRITE_RDF_STATISTICS_OPTION_SHORT = "";
const static inline std::string WRITE_RDF_STATISTICS_OPTION_LONG =
    "write-rdf-statistics";
const static inline std::string WRITE_RDF_STATISTICS_OPTION_HELP =
    "Storing RDF statistics as .stats.json";

}  // namespace osm2rdf::config::constants

#endif  // OSM2RDF_CONFIG_CONSTANTS_H
