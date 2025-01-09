// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#include <time.h>

#include <iomanip>
#include <iostream>

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/Way.h"
#include "osm2rdf/ttl/Writer.h"

using osm2rdf::osm::constants::AREA_PRECISION;
using osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR;
using osm2rdf::ttl::constants::DATASET_ID;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__AS_WKT;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_CENTROID;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL;
using osm2rdf::ttl::constants::IRI__OSM2RDF__LENGTH;
using osm2rdf::ttl::constants::IRI__OSM2RDF_FACTS;
using osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__CONVEX_HULL;
using osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__ENVELOPE;
using osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__OBB;
using osm2rdf::ttl::constants::IRI__OSM2RDF_MEMBER__ID;
using osm2rdf::ttl::constants::IRI__OSM2RDF_MEMBER__POS;
using osm2rdf::ttl::constants::IRI__OSM2RDF_MEMBER__ROLE;
using osm2rdf::ttl::constants::IRI__OSM_NODE;
using osm2rdf::ttl::constants::IRI__OSM_RELATION;
using osm2rdf::ttl::constants::IRI__OSM_TAG;
using osm2rdf::ttl::constants::IRI__OSM_WAY;
using osm2rdf::ttl::constants::IRI__OSMMETA_CHANGESET;
using osm2rdf::ttl::constants::IRI__OSMMETA_TIMESTAMP;
using osm2rdf::ttl::constants::IRI__OSMMETA_USER;
using osm2rdf::ttl::constants::IRI__OSMMETA_VERSION;
using osm2rdf::ttl::constants::IRI__OSMMETA_VISIBLE;
using osm2rdf::ttl::constants::IRI__OSMWAY_IS_CLOSED;
using osm2rdf::ttl::constants::IRI__OSMWAY_NEXT_NODE;
using osm2rdf::ttl::constants::IRI__OSMWAY_NEXT_NODE_DISTANCE;
using osm2rdf::ttl::constants::IRI__OSMWAY_NODE;
using osm2rdf::ttl::constants::IRI__OSMWAY_NODE_COUNT;
using osm2rdf::ttl::constants::IRI__OSMWAY_UNIQUE_NODE_COUNT;
using osm2rdf::ttl::constants::IRI__RDF_TYPE;
using osm2rdf::ttl::constants::IRI__XSD_DATE;
using osm2rdf::ttl::constants::IRI__XSD_DATE_TIME;
using osm2rdf::ttl::constants::IRI__XSD_DECIMAL;
using osm2rdf::ttl::constants::IRI__XSD_DOUBLE;
using osm2rdf::ttl::constants::IRI__XSD_INTEGER;
using osm2rdf::ttl::constants::IRI__XSD_YEAR;
using osm2rdf::ttl::constants::IRI__XSD_YEAR_MONTH;
using osm2rdf::ttl::constants::LITERAL__FALSE;
using osm2rdf::ttl::constants::LITERAL__TRUE;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_GEOM;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_META;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG;
using osm2rdf::ttl::constants::NAMESPACE__OSM_NODE;
using osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION;
using osm2rdf::ttl::constants::NAMESPACE__OSM_TAG;
using osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
using osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY;
using osm2rdf::ttl::constants::NODE_NAMESPACE;
using osm2rdf::ttl::constants::RELATION_NAMESPACE;
using osm2rdf::ttl::constants::WAY_NAMESPACE;

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::FactHandler<W>::FactHandler(const osm2rdf::config::Config& config,
                                          osm2rdf::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::area(const osm2rdf::osm::Area& area) {
  const std::string& subj = _writer->generateIRI(
      area.fromWay() ? WAY_NAMESPACE[_config.sourceDataset]
                     : RELATION_NAMESPACE[_config.sourceDataset],
      area.objId());

  const std::string& geomObj = _writer->generateIRIUnsafe(
      NAMESPACE__OSM2RDF_GEOM, DATASET_ID[_config.sourceDataset] + "_" +
                                   (area.fromWay() ? "way" : "rel") + "area_" +
                                   std::to_string(area.objId()));

  _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);

  if (area.geom().size() == 1) {
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, area.geom()[0]);
  } else {
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, area.geom());
  }

  if (_config.addCentroids) {
    const std::string& centroidObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM, DATASET_ID[_config.sourceDataset] +
                                     "_area_centroid_" +
                                     std::to_string(area.id()));
    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
    writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT, area.centroid());
  }
  writeGeometry(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL, area.convexHull());
  writeBox(subj, IRI__OSM2RDF_GEOM__ENVELOPE, area.envelope());
  writeGeometry(subj, IRI__OSM2RDF_GEOM__OBB, area.orientedBoundingBox());

  // Increase default precision as areas in regbez freiburg have a 0 area
  // otherwise.
  _writer->writeLiteralTripleUnsafe(
      subj, _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "area"),
      ::util::formatFloat(area.geomArea(), AREA_PRECISION),
      "^^" + IRI__XSD_DOUBLE);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::node(const osm2rdf::osm::Node& node) {
  const std::string& subj =
      _writer->generateIRI(NODE_NAMESPACE[_config.sourceDataset], node.id());

  _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_NODE);
  // Meta
  writeMeta(subj, node);

  // Tags
  writeTagList(subj, node.tags());

  const std::string& geomObj = _writer->generateIRIUnsafe(
      NAMESPACE__OSM2RDF_GEOM,
      DATASET_ID[_config.sourceDataset] + "_node_" + std::to_string(node.id()));

  _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);
  writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, node.geom());

  if (_config.addCentroids) {
    const std::string& centroidObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM, DATASET_ID[_config.sourceDataset] +
                                     "_node_centroid_" +
                                     std::to_string(node.id()));
    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
    writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT, node.geom());
  }

  const auto& hullWKT = ::util::geo::getWKT(
      ::util::geo::DPolygon{{node.geom()}, {}}, _config.wktPrecision);

  _writer->writeLiteralTripleUnsafe(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL,
                                    hullWKT,
                                    "^^" + IRI__GEOSPARQL__WKT_LITERAL);
  writeBox(subj, IRI__OSM2RDF_GEOM__ENVELOPE,
           ::util::geo::DBox{node.geom(), node.geom()});
  _writer->writeLiteralTripleUnsafe(subj, IRI__OSM2RDF_GEOM__OBB, hullWKT,
                                    "^^" + IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::relation(
    const osm2rdf::osm::Relation& relation) {
  const std::string& subj = _writer->generateIRI(
      RELATION_NAMESPACE[_config.sourceDataset], relation.id());

  _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_RELATION);
  // Meta
  writeMeta(subj, relation);
  // Tags
  writeTagList(subj, relation.tags());

  size_t inRelPos = 0;
  for (const auto& member : relation.members()) {
    std::string type;
    switch (member.type()) {
      case osm2rdf::osm::RelationMemberType::NODE:
        type = NODE_NAMESPACE[_config.sourceDataset];
        break;
      case osm2rdf::osm::RelationMemberType::RELATION:
        type = RELATION_NAMESPACE[_config.sourceDataset];
        break;
      case osm2rdf::osm::RelationMemberType::WAY:
        type = WAY_NAMESPACE[_config.sourceDataset];
        break;
      default:
        continue;
    }

    const std::string& role = member.role();
    const std::string& blankNode = _writer->generateBlankNode();
    _writer->writeTriple(
        subj, _writer->generateIRIUnsafe(NAMESPACE__OSM_RELATION, "member"),
        blankNode);

    _writer->writeTriple(blankNode, IRI__OSM2RDF_MEMBER__ID,
                         _writer->generateIRI(type, member.id()));
    _writer->writeTriple(blankNode, IRI__OSM2RDF_MEMBER__ROLE,
                         _writer->generateLiteral(role));
    _writer->writeLiteralTripleUnsafe(blankNode, IRI__OSM2RDF_MEMBER__POS,
                                      std::to_string(inRelPos++),
                                      "^^" + IRI__XSD_INTEGER);
  }

  if (relation.hasGeometry()) {
    const std::string& geomObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM, DATASET_ID[_config.sourceDataset] +
                                     "_relation_" +
                                     std::to_string(relation.id()));

    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, relation.geom());

    if (_config.addCentroids) {
      const std::string& centroidObj = _writer->generateIRIUnsafe(
          NAMESPACE__OSM2RDF_GEOM, DATASET_ID[_config.sourceDataset] +
                                       "_relation_centroid_" +
                                       std::to_string(relation.id()));
      _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
      writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT, relation.centroid());
    }
    writeGeometry(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL, relation.convexHull());
    writeBox(subj, osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__ENVELOPE,
             relation.envelope());
    writeGeometry(subj, IRI__OSM2RDF_GEOM__OBB, relation.orientedBoundingBox());

    _writer->writeTriple(
        subj,
        _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "completeGeometry"),
        relation.hasCompleteGeometry()
            ? osm2rdf::ttl::constants::LITERAL__TRUE
            : osm2rdf::ttl::constants::LITERAL__FALSE);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::way(const osm2rdf::osm::Way& way) {
  const std::string& subj =
      _writer->generateIRI(WAY_NAMESPACE[_config.sourceDataset], way.id());

  _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_WAY);
  // Meta
  writeMeta(subj, way);
  // Tags
  writeTagList(subj, way.tags());

  if (_config.addWayNodeOrder && way.nodes().size()) {
    size_t wayOrder = 0;
    std::string lastBlankNode;
    auto lastNode = way.nodes().front();
    for (const auto& node : way.nodes()) {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(subj, IRI__OSMWAY_NODE, blankNode);

      _writer->writeTriple(
          blankNode, osm2rdf::ttl::constants::IRI__OSMWAY_NODE,
          _writer->generateIRI(NODE_NAMESPACE[_config.sourceDataset],
                               node.id()));

      _writer->writeLiteralTripleUnsafe(blankNode, IRI__OSM2RDF_MEMBER__POS,
                                        std::to_string(wayOrder++),
                                        "^^" + IRI__XSD_INTEGER);

      if (_config.addWayNodeSpatialMetadata && !lastBlankNode.empty()) {
        _writer->writeTriple(
            lastBlankNode, IRI__OSMWAY_NEXT_NODE,
            _writer->generateIRI(NODE_NAMESPACE[_config.sourceDataset],
                                 node.id()));
        // Haversine distance
        const double distanceLat =
            (node.geom().getY() - lastNode.geom().getY()) *
            osm2rdf::osm::constants::DEGREE;
        const double distanceLon =
            (node.geom().getX() - lastNode.geom().getX()) *
            osm2rdf::osm::constants::DEGREE;
        const double haversine =
            (sin(distanceLat / 2) * sin(distanceLat / 2)) +
            (sin(distanceLon / 2) * sin(distanceLon / 2) *
             cos(lastNode.geom().getY() * osm2rdf::osm::constants::DEGREE) *
             cos(node.geom().getY() * osm2rdf::osm::constants::DEGREE));
        const double distance = osm2rdf::osm::constants::EARTH_RADIUS_KM *
                                osm2rdf::osm::constants::METERS_IN_KM * 2 *
                                asin(sqrt(haversine));
        _writer->writeLiteralTripleUnsafe(
            lastBlankNode, IRI__OSMWAY_NEXT_NODE_DISTANCE,
            std::to_string(distance), "^^" + IRI__XSD_DECIMAL);
      }
      lastBlankNode = blankNode;
      lastNode = node;
    }
  }

  size_t numUniquePoints = way.geom().size();

  if (_config.addAreaWayLinestrings || !way.isArea()) {
    const std::string& geomObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF, "way_" + std::to_string(way.id()));

    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, way.geom());
  }

  if (!way.isArea()) {
    // only write these triples if the way is not an area, otherwise they
    // are already written in the area handler
    if (_config.addCentroids) {
      const std::string& centroidObj = _writer->generateIRIUnsafe(
          NAMESPACE__OSM2RDF_GEOM, DATASET_ID[_config.sourceDataset] +
                                       "_way_centroid_" +
                                       std::to_string(way.id()));
      _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
      writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT, way.centroid());
    }
    writeGeometry(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL, way.convexHull());
    writeBox(subj, IRI__OSM2RDF_GEOM__ENVELOPE, way.envelope());
    writeGeometry(subj, IRI__OSM2RDF_GEOM__OBB, way.orientedBoundingBox());
  }

  if (_config.addWayMetadata) {
    _writer->writeTriple(subj, IRI__OSMWAY_IS_CLOSED,
                         way.closed() ? LITERAL__TRUE : LITERAL__FALSE);
    _writer->writeLiteralTripleUnsafe(subj, IRI__OSMWAY_NODE_COUNT,
                                      std::to_string(way.nodes().size()),
                                      "^^" + IRI__XSD_INTEGER);
    _writer->writeLiteralTripleUnsafe(subj, IRI__OSMWAY_UNIQUE_NODE_COUNT,
                                      std::to_string(numUniquePoints),
                                      "^^" + IRI__XSD_INTEGER);
  }

  _writer->writeLiteralTripleUnsafe(
      subj, IRI__OSM2RDF__LENGTH, std::to_string(::util::geo::len(way.geom())),
      "^^" + osm2rdf::ttl::constants::IRI__XSD_DOUBLE);
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
void osm2rdf::osm::FactHandler<W>::writeGeometry(const std::string& subj,
                                                 const std::string& pred,
                                                 const G& geom) {
  if (_config.simplifyWKT > 0 &&
      ::util::geo::numPoints(geom) > _config.simplifyWKT) {
    G simplifiedGeom;
    auto perimeter_or_length = ::util::geo::len(geom);
    do {
      simplifiedGeom = ::util::geo::simplify(geom, BASE_SIMPLIFICATION_FACTOR *
                                                       perimeter_or_length *
                                                       _config.wktDeviation);
      perimeter_or_length /= 2;
    } while ((::util::geo::empty(simplifiedGeom)) &&
             perimeter_or_length >= BASE_SIMPLIFICATION_FACTOR);
    _writer->writeLiteralTripleUnsafe(
        subj, pred, ::util::geo::getWKT(simplifiedGeom, _config.wktPrecision),
        "^^" + IRI__GEOSPARQL__WKT_LITERAL);
  } else {
    _writer->writeLiteralTripleUnsafe(
        subj, pred, ::util::geo::getWKT(geom, _config.wktPrecision),
        "^^" + IRI__GEOSPARQL__WKT_LITERAL);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeBox(
    const std::string& subj, const std::string& pred,
    const ::util::geo::Box<double>& box) {
  // Box can not be simplified -> output directly.
  _writer->writeLiteralTripleUnsafe(
      subj, pred, ::util::geo::getWKT(box, _config.wktPrecision),
      "^^" + IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
template <typename T>
void osm2rdf::osm::FactHandler<W>::writeMeta(const std::string& subj,
                                             const T& object) {
  if (!_config.addOsmMetadata) return;

  // avoid writing empty changeset IDs, drop entire triple
  if (object.changeset() != 0) {
    _writer->writeTriple(
        subj, IRI__OSMMETA_CHANGESET,
        _writer->generateLiteralUnsafe(std::to_string(object.changeset()),
                                       "^^" + IRI__XSD_INTEGER));
  }

  writeSecondsAsISO(subj, IRI__OSMMETA_TIMESTAMP, object.timestamp());

  // avoid writing empty users, drop entire triple
  if (!object.user().empty()) {
    _writer->writeTriple(subj, IRI__OSMMETA_USER,
                         _writer->generateLiteral(object.user(), ""));
  }

  _writer->writeTriple(
      subj, IRI__OSMMETA_VERSION,
      _writer->generateLiteralUnsafe(std::to_string(object.version()),
                                     "^^" + IRI__XSD_INTEGER));

  // only write visibility of it is false
  if (!object.visible()) {
    _writer->writeTriple(subj, IRI__OSMMETA_VISIBLE,
                         object.visible()
                             ? osm2rdf::ttl::constants::LITERAL__TRUE
                             : osm2rdf::ttl::constants::LITERAL__FALSE);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeTag(const std::string& subj,
                                            const osm2rdf::osm::Tag& tag) {
  const std::string& key = tag.first;
  const std::string& value = tag.second;
  if (key == "admin_level") {
    std::string rTrimmed;

    // right trim, left trim is done by strtoll
    auto end = std::find_if(value.rbegin(), value.rend(),
                            [](int c) { return std::isspace(c) == 0; });
    rTrimmed = value.substr(0, end.base() - value.begin());

    char* firstNonMatched;
    int64_t lvl = strtoll(rTrimmed.c_str(), &firstNonMatched,
                          osm2rdf::osm::constants::BASE10_BASE);

    // if integer, dump as xsd:integer
    if (firstNonMatched != rTrimmed.c_str() && (*firstNonMatched) == 0) {
      _writer->writeTriple(subj,
                           _writer->generateIRIUnsafe(NAMESPACE__OSM_TAG, key),
                           _writer->generateLiteralUnsafe(
                               std::to_string(lvl), "^^" + IRI__XSD_INTEGER));
    } else {
      _writer->writeUnsafeIRILiteralTriple(subj, NAMESPACE__OSM_TAG, key,
                                           value);
    }
  } else {
    auto check = _writer->checkPN_LOCAL(key);
    if (check == 0) {
      _writer->writeUnsafeIRILiteralTriple(subj, NAMESPACE__OSM_TAG, key,
                                           value);
    } else if (check == 1) {
      _writer->writeIRILiteralTriple(subj, NAMESPACE__OSM_TAG, key, value);
    } else {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(subj, IRI__OSM_TAG, blankNode);

      _writer->writeTriple(blankNode,
                           _writer->generateIRI(NAMESPACE__OSM_TAG, "key"),
                           _writer->generateLiteral(key));
      _writer->writeTriple(blankNode,
                           _writer->generateIRI(NAMESPACE__OSM_TAG, "value"),
                           _writer->generateLiteral(value));
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeTagList(
    const std::string& subj, const osm2rdf::osm::TagList& tags) {
  size_t tagTripleCount = 0;
  for (const auto& tag : tags) {
    const std::string& key = tag.first;
    const std::string& value = tag.second;
    // Special handling for ref tag splitting. Maybe generalize this...
    if (_config.semicolonTagKeys.find(key) != _config.semicolonTagKeys.end() &&
        value.find(';') != std::string::npos) {
      size_t end;
      size_t start = 0;
      while ((end = value.find(';', start)) != std::string::npos) {
        const std::string& partialValue = value.substr(start, (end - start));
        writeTag(subj, osm2rdf::osm::Tag(key, partialValue));
        tagTripleCount++;
        start = end + 1;
      };
      const std::string& partialValue = value.substr(start, value.size());
      writeTag(subj, osm2rdf::osm::Tag(key, partialValue));
      tagTripleCount++;
    } else {
      writeTag(subj, tag);
      tagTripleCount++;
    }

    // Handling for wiki tags
    if (!_config.skipWikiLinks &&
        (key == "wikidata" || hasSuffix(key, ":wikidata"))) {
      // Only take first wikidata entry if ; is found
      std::string valueTmp = value;
      const auto end = valueTmp.find(';');
      if (end != std::string::npos) {
        valueTmp = valueTmp.erase(end);
      }
      // Remove all but Q and digits to ensure Qdddddd format
      valueTmp.erase(
          remove_if(valueTmp.begin(), valueTmp.end(),
                    [](char chr) { return (chr != 'Q' && isdigit(chr) == 0); }),
          valueTmp.end());

      _writer->writeTriple(
          subj, _writer->generateIRI(NAMESPACE__OSM2RDF_TAG, key),
          _writer->generateIRI(NAMESPACE__WIKIDATA_ENTITY, valueTmp));
      tagTripleCount++;
    }
    if (!_config.skipWikiLinks &&
        (key == "wikipedia" || hasSuffix(key, ":wikipedia"))) {
      const auto pos = value.find(':');
      if (pos != std::string::npos) {
        const std::string& lang = value.substr(0, pos);
        const std::string& entry = value.substr(pos + 1);
        _writer->writeTriple(
            subj, _writer->generateIRI(NAMESPACE__OSM2RDF_TAG, key),
            _writer->generateIRI("https://" + lang + ".wikipedia.org/wiki/",
                                 entry));
        tagTripleCount++;
      } else {
        _writer->writeTriple(
            subj, _writer->generateIRI(NAMESPACE__OSM2RDF_TAG, key),
            _writer->generateIRI("https://www.wikipedia.org/wiki/", value));
        tagTripleCount++;
      }
    }
    if (key == "start_date" || key == "end_date") {
      // Abort if non digit and not -
      if (std::any_of(value.cbegin(), value.cend(),
                      [](char c) { return isdigit(c) == 0 && c != '-'; })) {
        continue;
      }

      // Skip if empty
      if (value.empty()) {
        continue;
      }
      // Skip if only '-'
      size_t minusCount = std::count(value.begin(), value.end(), '-');
      if (minusCount == value.size()) {
        continue;
      }

      std::string newValue;
      newValue.reserve(value.size());
      std::ostringstream tmp;
      tmp << std::setfill('0');

      size_t last = 0;
      size_t next;
      auto resultType = 0;
      for (size_t i = 0; i < (minusCount + 1); ++i) {
        next = value.find('-', last);
        if (i == 0 && next == 0) {
          newValue += '-';
          last = next + 1;
          continue;
        }
        auto val = std::atoi(value.substr(last, next - last).c_str());

        // basic validity checks according to ISO 8601
        if (resultType == 1 && (val < 1 || val > 12)) {
          resultType = 9;  // error
          break;
        }

        if (resultType == 2 && (val < 1 || val > 31)) {
          resultType = 9;  // error
          break;
        }

        tmp << std::setw(resultType == 0 ? 4 : 2) << std::dec << val;
        newValue += tmp.str().substr(0, resultType == 0 ? 4 : 2) + '-';
        tmp.seekp(0);
        resultType++;
        last = next + 1;
      }
      if (resultType > 3) {
        // Invalid length
        continue;
      }
      std::string typeString[3] = {IRI__XSD_YEAR, IRI__XSD_YEAR_MONTH,
                                   IRI__XSD_DATE};
      _writer->writeTriple(
          subj, _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF_TAG, key),
          _writer->generateLiteralUnsafe(
              newValue.substr(0, newValue.size() - 1),
              "^^" + typeString[resultType - 1]));
    }
  }
  _writer->writeTriple(
      subj, _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "facts"),
      _writer->generateLiteralUnsafe(std::to_string(tagTripleCount),
                                     "^^" + IRI__XSD_INTEGER));
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeSecondsAsISO(const std::string& subj,
                                                     const std::string& pred,
                                                     const std::time_t& time) {
  char out[25];

  struct tm t;
  strftime(out, 25, "%Y-%m-%dT%X", gmtime_r(&time, &t));

  _writer->writeLiteralTripleUnsafe(subj, pred, out, "^^" + IRI__XSD_DATE_TIME);
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::FactHandler<W>::hasSuffix(const std::string& subj,
                                             const std::string& suffix) const {
  if (subj.size() < suffix.size()) {
    return false;
  }
  return strcmp(subj.c_str() + subj.size() - suffix.size(), suffix.c_str()) ==
         0;
}

// ____________________________________________________________________________
template class osm2rdf::osm::FactHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::FactHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::FactHandler<osm2rdf::ttl::format::QLEVER>;
