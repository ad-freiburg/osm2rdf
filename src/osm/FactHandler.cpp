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

#include "osm2rdf/osm/FactHandler.h"

#include <time.h>

#include <iomanip>
#include <iostream>

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/osm/LocationHandler.h"
#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/Way.h"
#include "osm2rdf/ttl/Writer.h"

using osm2rdf::osm::constants::AREA_PRECISION;
using osm2rdf::osm::constants::LENGTH_PRECISION;
using osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR;
using osm2rdf::ttl::constants::CHANGESET_NAMESPACE;
using osm2rdf::ttl::constants::DATASET_ID;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__AS_WKT;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_CENTROID;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL;
using osm2rdf::ttl::constants::IRI__OSM2RDF__FACTS;
using osm2rdf::ttl::constants::IRI__OSM2RDF__LENGTH;
using osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__CONVEX_HULL;
using osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__ENVELOPE;
using osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__OBB;
using osm2rdf::ttl::constants::IRI__OSM__CHANGESET;
using osm2rdf::ttl::constants::IRI__OSM__NODE;
using osm2rdf::ttl::constants::IRI__OSM__RELATION;
using osm2rdf::ttl::constants::IRI__OSM__TAG;
using osm2rdf::ttl::constants::IRI__OSM__USER;
using osm2rdf::ttl::constants::IRI__OSM__WAY;
using osm2rdf::ttl::constants::IRI__OSMMETA__CHANGESET;
using osm2rdf::ttl::constants::IRI__OSMMETA__TIMESTAMP;
using osm2rdf::ttl::constants::IRI__OSMMETA__UID;
using osm2rdf::ttl::constants::IRI__OSMMETA__USER;
using osm2rdf::ttl::constants::IRI__OSMMETA__VERSION;
using osm2rdf::ttl::constants::IRI__OSMMETA__VISIBLE;
using osm2rdf::ttl::constants::IRI__OSMREL__MEMBER_ID;
using osm2rdf::ttl::constants::IRI__OSMREL__MEMBER_POS;
using osm2rdf::ttl::constants::IRI__OSMREL__MEMBER_ROLE;
using osm2rdf::ttl::constants::IRI__OSMWAY__IS_CLOSED;
using osm2rdf::ttl::constants::IRI__OSMWAY__NEXT_NODE;
using osm2rdf::ttl::constants::IRI__OSMWAY__NEXT_NODE_DISTANCE;
using osm2rdf::ttl::constants::IRI__OSMWAY__NODE;
using osm2rdf::ttl::constants::IRI__OSMWAY__NODE_COUNT;
using osm2rdf::ttl::constants::IRI__OSMWAY__UNIQUE_NODE_COUNT;
using osm2rdf::ttl::constants::IRI__RDF__TYPE;
using osm2rdf::ttl::constants::IRI__XSD__DATE;
using osm2rdf::ttl::constants::IRI__XSD__DATE_TIME;
using osm2rdf::ttl::constants::IRI__XSD__DECIMAL;
using osm2rdf::ttl::constants::IRI__XSD__DOUBLE;
using osm2rdf::ttl::constants::IRI__XSD__INTEGER;
using osm2rdf::ttl::constants::IRI__XSD__YEAR;
using osm2rdf::ttl::constants::IRI__XSD__YEAR_MONTH;
using osm2rdf::ttl::constants::IRI_PREFIX_NODE_TAGGED;
using osm2rdf::ttl::constants::LITERAL__FALSE;
using osm2rdf::ttl::constants::LITERAL__TRUE;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_GEOM;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_META;
using osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_TAG;
using osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION;
using osm2rdf::ttl::constants::NAMESPACE__OSM_TAG;
using osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
using osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY;
using osm2rdf::ttl::constants::NODE_NAMESPACE;
using osm2rdf::ttl::constants::NODE_NAMESPACE_TAGGED;
using osm2rdf::ttl::constants::NODE_NAMESPACE_UNTAGGED;
using osm2rdf::ttl::constants::RELATION_NAMESPACE;
using osm2rdf::ttl::constants::WAY_NAMESPACE;

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::FactHandler<W>::FactHandler(const osm2rdf::config::Config& config,
                                          osm2rdf::ttl::Writer<W>* writer)
    : _config(config), _writer(writer), _locationHandler(nullptr) {
  _separateUntaggedNodePrefixes = _config.iriPrefixForUntaggedNodes !=
                                  IRI_PREFIX_NODE_TAGGED[_config.sourceDataset];
  _datasetId = DATASET_ID[_config.sourceDataset];
  _relNamespace = RELATION_NAMESPACE[_config.sourceDataset];
  _wayNamespace = WAY_NAMESPACE[_config.sourceDataset];
  _changesetNamespace = CHANGESET_NAMESPACE[_config.sourceDataset];
  _iriXSDDouble = "^^" + IRI__XSD__DOUBLE;
  _iriXSDInteger = "^^" + IRI__XSD__INTEGER;
  _iriWKTLiteral = "^^" + IRI__GEOSPARQL__WKT_LITERAL;
  _tagTripleCountIRI = _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "facts");
  _areaIRI = writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "area");
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::setLocationHandler(
    osm2rdf::osm::LocationHandler* locationHandler) {
  _locationHandler = locationHandler;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::area(const osm2rdf::osm::Area& area) {
  const std::string& sid = std::to_string(area.objId());
  const std::string& subj =
      _writer->generateIRI(area.fromWay() ? _wayNamespace : _relNamespace, sid);

  const std::string& geomObj = _writer->generateIRIUnsafe(
      NAMESPACE__OSM2RDF_GEOM,
      _datasetId + (area.fromWay() ? "way_" : "rel_") + sid);

  _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);

  if (area.geom().size() == 1) {
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, area.geom()[0]);
  } else {
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, area.geom());
  }

  if (_config.addCentroid) {
    const std::string& centroidObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM,
        _datasetId + (area.fromWay() ? "way_" : "rel_") + "centroid_" + sid);
    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
    writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT, area.centroid());
  }

  if (_config.addConvexHull) {
    writeGeometry(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL, area.convexHull());
  }

  if (_config.addEnvelope) {
    writeBox(subj, IRI__OSM2RDF_GEOM__ENVELOPE, area.envelope());
  }

  if (_config.addObb) {
    writeGeometry(subj, IRI__OSM2RDF_GEOM__OBB, area.orientedBoundingBox());
  }

  // Increase default precision as areas in regbez freiburg have a 0 area
  // otherwise.
  _writer->writeLiteralTripleUnsafe(
      subj, _areaIRI, ::util::formatFloat(area.geomArea(), AREA_PRECISION),
      _iriXSDDouble);

  if (!area.fromWay()) {
    // for areas from relations, always write hasCompleteGeometry true for
    // consistency with non-area relations
    _writer->writeTriple(
        subj,
        _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "hasCompleteGeometry"),
            osm2rdf::ttl::constants::LITERAL__TRUE
            );
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::node(const osmium::Node& node) {
  const bool untagged = node.tags().empty();
  const std::string& sid = std::to_string(node.id());

  const std::string& subj =
      !_separateUntaggedNodePrefixes
          ? _writer->generateIRI(NODE_NAMESPACE[_config.sourceDataset], sid)
          : (untagged ? _writer->generateIRI(
                            NODE_NAMESPACE_UNTAGGED[_config.sourceDataset], sid)
                      : _writer->generateIRI(
                            NODE_NAMESPACE_TAGGED[_config.sourceDataset], sid));

  _writer->writeTriple(subj, IRI__RDF__TYPE, IRI__OSM__NODE);

  // Meta
  writeMeta(subj, node);

  // Tags
  writeTagList(subj, node.tags());

  if (node.location().valid()) {
    const std::string& geomObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM, _datasetId +
                                     (!_separateUntaggedNodePrefixes ? "node_"
                                      : untagged ? "node_untagged_"
                                                 : "node_tagged_") +
                                     sid);

    auto geom = ::util::geo::DPoint{node.location().lon_without_check(),
                                    node.location().lat_without_check()};

    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, geom);

    if (_config.addCentroid) {
      const std::string& centroidObj = _writer->generateIRIUnsafe(
          NAMESPACE__OSM2RDF_GEOM, _datasetId + "_node_centroid_" + sid);
      _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
      writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT, geom);
    }

    if (_config.addObb || _config.addConvexHull) {
      const auto& hullWKT = ::util::geo::getWKT(
          ::util::geo::DPolygon{{geom}, {}}, _config.wktPrecision);
      if (_config.addObb) {
        _writer->writeLiteralTripleUnsafe(subj, IRI__OSM2RDF_GEOM__OBB, hullWKT,
                                          _iriWKTLiteral);
      }
      if (_config.addConvexHull) {
        _writer->writeLiteralTripleUnsafe(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL,
                                          hullWKT, _iriWKTLiteral);
      }
    }

    if (_config.addEnvelope) {
      writeBox(subj, IRI__OSM2RDF_GEOM__ENVELOPE,
               ::util::geo::DBox{geom, geom});
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::relation(
    const osm2rdf::osm::Relation& relation) {
  const std::string& sid = std::to_string(relation.id());
  const std::string& subj = _writer->generateIRI(_relNamespace, sid);

  _writer->writeTriple(subj, IRI__RDF__TYPE, IRI__OSM__RELATION);
  // Meta
  writeMeta(subj, relation);

  // Tags
  writeTagList(subj, relation.tags());

  if (_config.addMemberTriples && relation.members().size()) {
    size_t inRelPos = 0;
    for (const auto& member : relation.members()) {
      std::string type;
      switch (member.type()) {
        case osmium::item_type::node:
          if (!_separateUntaggedNodePrefixes) {
            type = NODE_NAMESPACE[_config.sourceDataset];
          } else if (_locationHandler->get_node_is_tagged(
                         member.positive_ref())) {
            type = NODE_NAMESPACE_TAGGED[_config.sourceDataset];
          } else {
            type = NODE_NAMESPACE_UNTAGGED[_config.sourceDataset];
          }
          break;
        case osmium::item_type::relation:
          type = _relNamespace;
          break;
        case osmium::item_type::way:
          type = _wayNamespace;
          break;
        default:
          continue;
      }

      std::string role = member.role();
      if (role.empty()) role = "member";
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(
          subj, _writer->generateIRIUnsafe(NAMESPACE__OSM_RELATION, "member"),
          blankNode);

      _writer->writeTriple(blankNode, IRI__OSMREL__MEMBER_ID,
                           _writer->generateIRI(type, member.positive_ref()));
      _writer->writeTriple(blankNode, IRI__OSMREL__MEMBER_ROLE,
                           _writer->generateLiteral(role));
      _writer->writeLiteralTripleUnsafe(blankNode, IRI__OSMREL__MEMBER_POS,
                                        std::to_string(inRelPos++),
                                        _iriXSDInteger);
    }
  }

  if (relation.hasGeometry()) {
    const std::string& geomObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM, _datasetId + "rel_" + sid);

    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, relation.geom());

    if (_config.addCentroid) {
      const std::string& centroidObj = _writer->generateIRIUnsafe(
          NAMESPACE__OSM2RDF_GEOM, _datasetId + "rel_centroid_" + sid);
      _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
      writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT,
                    ::util::geo::centroid(relation.geom()));
    }

    if (_config.addConvexHull) {
      writeGeometry(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL,
                    ::util::geo::convexHull(relation.geom()));
    }

    if (_config.addEnvelope) {
      writeBox(subj, osm2rdf::ttl::constants::IRI__OSM2RDF_GEOM__ENVELOPE,
               ::util::geo::getBoundingBox(relation.geom()));
    }

    if (_config.addObb) {
      writeGeometry(subj, IRI__OSM2RDF_GEOM__OBB,
                    ::util::geo::convexHull(
                        ::util::geo::getOrientedEnvelope(relation.geom())));
    }

    _writer->writeTriple(
        subj,
        _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF, "hasCompleteGeometry"),
        relation.hasCompleteGeometry()
            ? osm2rdf::ttl::constants::LITERAL__TRUE
            : osm2rdf::ttl::constants::LITERAL__FALSE);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::way(const osm2rdf::osm::Way& way) {
  const std::string& sid = std::to_string(way.id());
  const std::string& subj = _writer->generateIRIUnsafe(_wayNamespace, sid);

  _writer->writeTriple(subj, IRI__RDF__TYPE, IRI__OSM__WAY);

  // Meta
  writeMeta(subj, way);

  // Tags
  writeTagList(subj, way.tags());

  if (_config.addMemberTriples && way.nodes().size()) {
    size_t wayOrder = 0;
    std::string lastBlankNode;
    auto lastNode = way.nodes().front();
    for (const auto& node : way.nodes()) {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(subj, IRI__OSMWAY__NODE, blankNode);

      std::string nodeNamespace;
      if (_config.iriPrefixForUntaggedNodes ==
          IRI_PREFIX_NODE_TAGGED[_config.sourceDataset]) {
        nodeNamespace = NODE_NAMESPACE[_config.sourceDataset];
      } else if (_locationHandler->get_node_is_tagged(node.positive_ref())) {
        nodeNamespace = NODE_NAMESPACE_TAGGED[_config.sourceDataset];
      } else {
        nodeNamespace = NODE_NAMESPACE_UNTAGGED[_config.sourceDataset];
      }

      _writer->writeTriple(
          blankNode, osm2rdf::ttl::constants::IRI__OSMWAY__MEMBER_ID,
          _writer->generateIRI(nodeNamespace, node.positive_ref()));

      _writer->writeLiteralTripleUnsafe(
          blankNode, osm2rdf::ttl::constants::IRI__OSMWAY__MEMBER_POS,
          std::to_string(wayOrder++), _iriXSDInteger);

      if (_config.addWayNodeSpatialMetadata && !lastBlankNode.empty() &&
          node.location().valid() && lastNode.location().valid()) {
        _writer->writeTriple(
            lastBlankNode, IRI__OSMWAY__NEXT_NODE,
            _writer->generateIRI(nodeNamespace, node.positive_ref()));
        // Haversine distance
        const double distanceLat = (node.location().lat_without_check() -
                                    lastNode.location().lat_without_check()) *
                                   osm2rdf::osm::constants::DEGREE;
        const double distanceLon = (node.location().lon_without_check() -
                                    lastNode.location().lon_without_check()) *
                                   osm2rdf::osm::constants::DEGREE;
        const double haversine =
            (sin(distanceLat / 2) * sin(distanceLat / 2)) +
            (sin(distanceLon / 2) * sin(distanceLon / 2) *
             cos(lastNode.location().lat() * osm2rdf::osm::constants::DEGREE) *
             cos(node.location().lat() * osm2rdf::osm::constants::DEGREE));
        const double distance = osm2rdf::osm::constants::EARTH_RADIUS_KM *
                                osm2rdf::osm::constants::METERS_IN_KM * 2 *
                                asin(sqrt(haversine));
        _writer->writeLiteralTripleUnsafe(
            lastBlankNode, IRI__OSMWAY__NEXT_NODE_DISTANCE,
            std::to_string(distance), "^^" + IRI__XSD__DECIMAL);
      }
      lastBlankNode = blankNode;
      lastNode = node;
    }
  }

  const auto& wayGeom = way.geom();
  size_t numUniquePoints = wayGeom.size();

  if (_config.addAreaWayLinestrings || !way.isArea()) {
    const std::string& geomObj = _writer->generateIRIUnsafe(
        NAMESPACE__OSM2RDF_GEOM, _datasetId + "way_" + sid);

    _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_GEOMETRY, geomObj);
    writeGeometry(geomObj, IRI__GEOSPARQL__AS_WKT, wayGeom);
  }

  if (!way.isArea()) {
    // only write these triples if the way is not an area, otherwise they
    // are already written in the area handler
    if (_config.addCentroid) {
      const std::string& centroidObj = _writer->generateIRIUnsafe(
          NAMESPACE__OSM2RDF_GEOM, _datasetId + "way_centroid_" + sid);
      _writer->writeTriple(subj, IRI__GEOSPARQL__HAS_CENTROID, centroidObj);
      writeGeometry(centroidObj, IRI__GEOSPARQL__AS_WKT,
                    ::util::geo::centroid(wayGeom));
    }

    if (_config.addConvexHull) {
      writeGeometry(subj, IRI__OSM2RDF_GEOM__CONVEX_HULL,
                    ::util::geo::convexHull(wayGeom));
    }

    if (_config.addEnvelope) {
      writeBox(subj, IRI__OSM2RDF_GEOM__ENVELOPE,
               ::util::geo::getBoundingBox(wayGeom));
    }

    if (_config.addObb) {
      writeGeometry(
          subj, IRI__OSM2RDF_GEOM__OBB,
          ::util::geo::convexHull(::util::geo::getOrientedEnvelope(wayGeom)));
    }
  }

  if (_config.addWayMetadata) {
    _writer->writeTriple(subj, IRI__OSMWAY__IS_CLOSED,
                         way.closed() ? LITERAL__TRUE : LITERAL__FALSE);
    _writer->writeLiteralTripleUnsafe(subj, IRI__OSMWAY__NODE_COUNT,
                                      std::to_string(way.nodes().size()),
                                      _iriXSDInteger);
    _writer->writeLiteralTripleUnsafe(subj, IRI__OSMWAY__UNIQUE_NODE_COUNT,
                                      std::to_string(numUniquePoints),
                                      _iriXSDInteger);
  }

  _writer->writeLiteralTripleUnsafe(
      subj, IRI__OSM2RDF__LENGTH,
      ::util::formatFloat(::util::geo::latLngLen(way.geom()), LENGTH_PRECISION),
      _iriXSDDouble);
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
        _iriWKTLiteral);
  } else {
    _writer->writeLiteralTripleUnsafe(
        subj, pred, ::util::geo::getWKT(geom, _config.wktPrecision),
        _iriWKTLiteral);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeGeometry(const std::string& subj,
                                                 const std::string& pred,
                                                 const ::util::geo::DPoint& p) {
  // directly construct WKT on the output buffer

  _writer->write(subj);
  _writer->write(' ');
  _writer->write(pred);
  _writer->write(' ');

  _writer->write("\"POINT(");
  _writer->write(::util::formatFloat(p.getX(), _config.wktPrecision));
  _writer->write(' ');
  _writer->write(::util::formatFloat(p.getY(), _config.wktPrecision));
  _writer->write(')');
  _writer->write('"');
  _writer->write(_iriWKTLiteral);

  _writer->write(" .");
  _writer->writeNewLine();
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeBox(
    const std::string& subj, const std::string& pred,
    const ::util::geo::Box<double>& box) {
  // Box can not be simplified -> output directly.
  _writer->writeLiteralTripleUnsafe(
      subj, pred, ::util::geo::getWKT(box, _config.wktPrecision),
      _iriWKTLiteral);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeMeta(const std::string& subj,
                                             const osmium::Node& object) {
  if (!_config.addOsmMetadata) return;

  // avoid writing empty changeset IDs, drop entire triple
  if (object.changeset() != 0) {
    _writer->writeTriple(
        subj, IRI__OSMMETA__CHANGESET,
        _writer->generateIRI(_changesetNamespace, object.changeset()));
  }

  _writer->writeSecondsAsISO(subj, IRI__OSMMETA__TIMESTAMP,
                             object.timestamp().seconds_since_epoch());

  // avoid writing empty users, drop entire triple
  if (!object.user_is_anonymous() && strlen(object.user()) > 0) {
    _writer->writeTriple(subj, IRI__OSMMETA__USER,
                         _writer->generateLiteral(object.user(), ""));
  }

  // avoid writing empty user IDs, drop entire triple
  if (object.uid() != 0) {
    _writer->writeTriple(subj, IRI__OSMMETA__UID,
                         _writer->generateLiteralUnsafe(
                             std::to_string(object.uid()), _iriXSDInteger));
  }

  _writer->writeTriple(subj, IRI__OSMMETA__VERSION,
                       _writer->generateLiteralUnsafe(
                           std::to_string(object.version()), _iriXSDInteger));

  // only write visibility of it is false
  if (!object.visible()) {
    _writer->writeTriple(subj, IRI__OSMMETA__VISIBLE,
                         object.visible()
                             ? osm2rdf::ttl::constants::LITERAL__TRUE
                             : osm2rdf::ttl::constants::LITERAL__FALSE);
  }
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
        subj, IRI__OSMMETA__CHANGESET,
        _writer->generateIRI(_changesetNamespace, object.changeset()));
  }

  _writer->writeSecondsAsISO(subj, IRI__OSMMETA__TIMESTAMP, object.timestamp());

  // avoid writing empty users, drop entire triple
  if (!object.user().empty()) {
    _writer->writeTriple(subj, IRI__OSMMETA__USER,
                         _writer->generateLiteral(object.user(), ""));
  }

  // avoid writing empty user IDs, drop entire triple
  if (object.uid() != 0) {
    _writer->writeTriple(subj, IRI__OSMMETA__UID,
                         _writer->generateLiteralUnsafe(
                             std::to_string(object.uid()), _iriXSDInteger));
  }

  _writer->writeTriple(subj, IRI__OSMMETA__VERSION,
                       _writer->generateLiteralUnsafe(
                           std::to_string(object.version()), _iriXSDInteger));

  // only write visibility of it is false
  if (!object.visible()) {
    _writer->writeTriple(subj, IRI__OSMMETA__VISIBLE,
                         object.visible()
                             ? osm2rdf::ttl::constants::LITERAL__TRUE
                             : osm2rdf::ttl::constants::LITERAL__FALSE);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeTag(const std::string& subj,
                                            const char* key,
                                            const char* value) {
  if (strcmp(key, "admin_level") == 0) {
    const char* trimmed = value;
    while (*trimmed && std::isspace(*trimmed)) trimmed++;
    char* firstNonMatched;
    int64_t lvl = strtoll(trimmed, &firstNonMatched,
                          osm2rdf::osm::constants::BASE10_BASE);
    while (*firstNonMatched && std::isspace(*firstNonMatched))
      firstNonMatched++;

    // if integer, dump as xsd:integer
    if (firstNonMatched != trimmed && (*firstNonMatched) == 0) {
      _writer->writeTriple(
          subj, _writer->generateIRIUnsafe(NAMESPACE__OSM_TAG, key),
          _writer->generateLiteralUnsafe(std::to_string(lvl), _iriXSDInteger));
    } else {
      _writer->writeUnsafeIRILiteralTriple(
          subj.c_str(), NAMESPACE__OSM_TAG.c_str(), key, value);
    }
  } else {
    auto check = _writer->checkPN_LOCAL(key);
    if (check == 0) {
      _writer->writeUnsafeIRILiteralTriple(
          subj.c_str(), NAMESPACE__OSM_TAG.c_str(), key, value);
    } else if (check == 1) {
      _writer->writeIRILiteralTriple(subj, NAMESPACE__OSM_TAG, key, value);
    } else {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(subj, IRI__OSM__TAG, blankNode);

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
void osm2rdf::osm::FactHandler<W>::writeTagList(const std::string& subj,
                                                const osmium::TagList& tags) {
  size_t tagTripleCount = 0;
  for (const auto& tag : tags) {
    // replace whitespace in key with _
    char* key = const_cast<char*>(tag.key());  // not very nice, but safes copy
    for (char* c = key; *c; c++) {
      if (std::iswspace(*c)) *c = '_';
    }

    const char* value = tag.value();

    // Special handling for ref tag splitting. Maybe generalize this...
    if (_config.semicolonTagKeys.find(key) != _config.semicolonTagKeys.end() &&
        strchr(value, ';') != 0) {
      // very expensive, do this with less copying
      size_t end;
      size_t start = 0;
      std::string valueStr = value;
      while ((end = valueStr.find(';', start)) != std::string::npos) {
        const std::string& partialValue = valueStr.substr(start, (end - start));
        writeTag(subj, key, partialValue.c_str());
        tagTripleCount++;
        start = end + 1;
      };
      const std::string& partialValue = valueStr.substr(start, valueStr.size());
      writeTag(subj, key, partialValue.c_str());
      tagTripleCount++;
    } else {
      writeTag(subj, key, value);
      tagTripleCount++;
    }

    // Handling for wiki tags
    if (!_config.skipWikiLinks &&
        (strcmp(key, "wikidata") == 0 || hasSuffix(key, ":wikidata", 9))) {
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
        (strcmp(key, "wikipedia") == 0 || hasSuffix(key, ":wikipedia", 10))) {
      std::string valueTmp = value;
      const auto pos = valueTmp.find(':');
      if (pos != std::string::npos) {
        const std::string& lang = valueTmp.substr(0, pos);
        const std::string& entry = valueTmp.substr(pos + 1);
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

    if (strcmp(key, "start_date") == 0 || strcmp(key, "end_date") == 0) {
      std::string valueTmp = value;
      // Abort if non digit and not -
      if (std::any_of(valueTmp.cbegin(), valueTmp.cend(),
                      [](char c) { return isdigit(c) == 0 && c != '-'; })) {
        continue;
      }

      // Skip if empty
      if (valueTmp.empty()) {
        continue;
      }
      // Skip if only '-'
      size_t minusCount = std::count(valueTmp.begin(), valueTmp.end(), '-');
      if (minusCount == valueTmp.size()) {
        continue;
      }

      std::string newValue;
      newValue.reserve(valueTmp.size());
      std::ostringstream tmp;
      tmp << std::setfill('0');

      size_t last = 0;
      size_t next;
      auto resultType = 0;
      for (size_t i = 0; i < (minusCount + 1); ++i) {
        next = valueTmp.find('-', last);
        if (i == 0 && next == 0) {
          newValue += '-';
          last = next + 1;
          continue;
        }
        auto val = std::atoi(valueTmp.substr(last, next - last).c_str());

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
      std::string typeString[3] = {IRI__XSD__YEAR, IRI__XSD__YEAR_MONTH,
                                   IRI__XSD__DATE};
      _writer->writeTriple(
          subj, _writer->generateIRIUnsafe(NAMESPACE__OSM2RDF_TAG, key),
          _writer->generateLiteralUnsafe(
              newValue.substr(0, newValue.size() - 1),
              "^^" + typeString[resultType - 1]));
    }
  }
  if (tagTripleCount > 0 || _config.addZeroFactNumber) {
    _writer->writeTriple(subj, _tagTripleCountIRI,
                         _writer->generateLiteralUnsafe(
                             std::to_string(tagTripleCount), _iriXSDInteger));
  }
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::FactHandler<W>::hasSuffix(const char* subj,
                                             const char* suffix,
                                             size_t suffixSize) const {
  size_t subjSize = strlen(subj);
  if (subjSize < suffixSize) {
    return false;
  }
  return strcmp(subj + subjSize - suffixSize, suffix) == 0;
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
