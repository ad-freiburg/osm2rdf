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

#include <iomanip>
#include <iostream>

#include "boost/geometry.hpp"
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
using osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY;
using osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL;
using osm2rdf::ttl::constants::IRI__OSM_META__ENVELOPE;
using osm2rdf::ttl::constants::IRI__OSM_META__POS;
using osm2rdf::ttl::constants::IRI__OSM_NODE;
using osm2rdf::ttl::constants::IRI__OSM_RELATION;
using osm2rdf::ttl::constants::IRI__OSM_TAG;
using osm2rdf::ttl::constants::IRI__OSM_WAY;
using osm2rdf::ttl::constants::IRI__OSMWAY_IS_CLOSED;
using osm2rdf::ttl::constants::IRI__OSMWAY_NEXT_NODE;
using osm2rdf::ttl::constants::IRI__OSMWAY_NEXT_NODE_DISTANCE;
using osm2rdf::ttl::constants::IRI__OSMWAY_NODE;
using osm2rdf::ttl::constants::IRI__OSMWAY_NODE_COUNT;
using osm2rdf::ttl::constants::IRI__OSMWAY_UNIQUE_NODE_COUNT;
using osm2rdf::ttl::constants::IRI__RDF_TYPE;
using osm2rdf::ttl::constants::IRI__XSD_DECIMAL;
using osm2rdf::ttl::constants::IRI__XSD_DOUBLE;
using osm2rdf::ttl::constants::IRI__XSD_INTEGER;
using osm2rdf::ttl::constants::LITERAL__NO;
using osm2rdf::ttl::constants::LITERAL__YES;
using osm2rdf::ttl::constants::NAMESPACE__OSM;
using osm2rdf::ttl::constants::NAMESPACE__OSM_META;
using osm2rdf::ttl::constants::NAMESPACE__OSM_NODE;
using osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION;
using osm2rdf::ttl::constants::NAMESPACE__OSM_TAG;
using osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
using osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY;

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::FactHandler<W>::FactHandler(const osm2rdf::config::Config& config,
                                          osm2rdf::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::area(const osm2rdf::osm::Area& area) {
  const std::string& subj = _writer->generateIRI(
      area.fromWay() ? NAMESPACE__OSM_WAY : NAMESPACE__OSM_RELATION,
      area.objId());

  writeBoostGeometry(subj, IRI__GEOSPARQL__HAS_GEOMETRY, area.geom());

  if (_config.addAreaEnvelope) {
    writeBox(subj, IRI__OSM_META__ENVELOPE, area.envelope());
  }

  if (_config.addSortMetadata) {
    std::ostringstream tmp;
    // Increase default precision as areas in regbez freiburg have a 0 area
    // otherwise.
    tmp << std::fixed << std::setprecision(AREA_PRECISION) << area.geomArea();
    _writer->writeTriple(
        subj, _writer->generateIRIUnsafe(NAMESPACE__OSM_META, "area"),
        _writer->generateLiteralUnsafe(tmp.str(), "^^" + IRI__XSD_DOUBLE));
  }

  if (_config.addAreaEnvelopeRatio) {
    std::ostringstream tmp;
    tmp << std::fixed << (area.geomArea() / area.envelopeArea());
    _writer->writeTriple(
        subj,
        _writer->generateIRIUnsafe(NAMESPACE__OSM_META, "area_envelope_ratio"),
        _writer->generateLiteralUnsafe(tmp.str(), "^^" + IRI__XSD_DOUBLE));
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::node(const osm2rdf::osm::Node& node) {
  const std::string& subj =
      _writer->generateIRI(NAMESPACE__OSM_NODE, node.id());

  _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_NODE);

  writeBoostGeometry(subj, IRI__GEOSPARQL__HAS_GEOMETRY, node.geom());

  writeTagList(subj, node.tags());

  osm2rdf::geometry::Box envelope;
  boost::geometry::envelope(node.geom(), envelope);

  if (_config.addNodeEnvelope) {
    writeBox(subj, IRI__OSM_META__ENVELOPE, envelope);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::relation(
    const osm2rdf::osm::Relation& relation) {
  const std::string& subj =
      _writer->generateIRI(NAMESPACE__OSM_RELATION, relation.id());

  _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_RELATION);

  writeTagList(subj, relation.tags());

  size_t inRelPos = 0;
  for (const auto& member : relation.members()) {
    const std::string& role = member.role();
    if (_config.addRelationBorderMembers ||
        (role != "outer" && role != "inner")) {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(
          subj, _writer->generateIRIUnsafe(NAMESPACE__OSM_RELATION, "member"),
          blankNode);

      std::string type;
      switch (member.type()) {
        case osm2rdf::osm::RelationMemberType::NODE:
          type = NAMESPACE__OSM_NODE;
          break;
        case osm2rdf::osm::RelationMemberType::RELATION:
          type = NAMESPACE__OSM_RELATION;
          break;
        case osm2rdf::osm::RelationMemberType::WAY:
          type = NAMESPACE__OSM_WAY;
          break;
        default:
          type = NAMESPACE__OSM;
      }

      _writer->writeTriple(blankNode,
                           _writer->generateIRIUnsafe(NAMESPACE__OSM, "id"),
                           _writer->generateIRI(type, member.id()));
      _writer->writeTriple(blankNode,
                           _writer->generateIRIUnsafe(NAMESPACE__OSM, "role"),
                           _writer->generateLiteral(role, ""));
      _writer->writeTriple(
          blankNode, IRI__OSM_META__POS,
          _writer->generateLiteralUnsafe(std::to_string(inRelPos++),
                                         "^^" + IRI__XSD_INTEGER));
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::way(const osm2rdf::osm::Way& way) {
  const std::string& subj = _writer->generateIRI(NAMESPACE__OSM_WAY, way.id());

  _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_WAY);

  writeTagList(subj, way.tags());

  if (_config.addWayNodeOrder) {
    size_t wayOrder = 0;
    std::string lastBlankNode;
    auto lastNode = way.nodes().front();
    for (const auto& node : way.nodes()) {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(subj, IRI__OSMWAY_NODE, blankNode);

      _writer->writeTriple(
          blankNode, osm2rdf::ttl::constants::IRI__OSMWAY_NODE,
          _writer->generateIRI(NAMESPACE__OSM_NODE, node.id()));

      _writer->writeTriple(
          blankNode, IRI__OSM_META__POS,
          _writer->generateLiteralUnsafe(std::to_string(wayOrder++),
                                         "^^" + IRI__XSD_INTEGER));

      if (_config.addWayNodeGeometry) {
        const std::string& subj =
            _writer->generateIRI(NAMESPACE__OSM_NODE, node.id());

        _writer->writeTriple(subj, IRI__RDF_TYPE, IRI__OSM_NODE);

        writeBoostGeometry(subj, IRI__GEOSPARQL__HAS_GEOMETRY, node.geom());
      }

      if (_config.addWayNodeSpatialMetadata && !lastBlankNode.empty()) {
        _writer->writeTriple(
            lastBlankNode, IRI__OSMWAY_NEXT_NODE,
            _writer->generateIRI(NAMESPACE__OSM_NODE, node.id()));
        // Haversine distance
        const double distanceLat = (node.geom().y() - lastNode.geom().y()) *
                                   osm2rdf::osm::constants::DEGREE;
        const double distanceLon = (node.geom().x() - lastNode.geom().x()) *
                                   osm2rdf::osm::constants::DEGREE;
        const double haversine =
            (sin(distanceLat / 2) * sin(distanceLat / 2)) +
            (sin(distanceLon / 2) * sin(distanceLon / 2) *
             cos(lastNode.geom().y() * osm2rdf::osm::constants::DEGREE) *
             cos(node.geom().y() * osm2rdf::osm::constants::DEGREE));
        const double distance = osm2rdf::osm::constants::EARTH_RADIUS_KM *
                                osm2rdf::osm::constants::METERS_IN_KM * 2 *
                                asin(sqrt(haversine));
        _writer->writeTriple(
            lastBlankNode, IRI__OSMWAY_NEXT_NODE_DISTANCE,
            _writer->generateLiteralUnsafe(std::to_string(distance),
                                           "^^" + IRI__XSD_DECIMAL));
      }
      lastBlankNode = blankNode;
      lastNode = node;
    }
  }

  osm2rdf::geometry::Linestring locations{way.geom()};
  size_t numUniquePoints = locations.size();
  writeBoostGeometry(subj, IRI__GEOSPARQL__HAS_GEOMETRY, locations);

  if (_config.addWayEnvelope) {
    writeBox(subj, IRI__OSM_META__ENVELOPE, way.envelope());
  }

  if (_config.addWayMetadata) {
    _writer->writeTriple(subj, IRI__OSMWAY_IS_CLOSED,
                         way.closed() ? LITERAL__YES : LITERAL__NO);
    _writer->writeTriple(
        subj, IRI__OSMWAY_NODE_COUNT,
        _writer->generateLiteralUnsafe(std::to_string(way.nodes().size()),
                                       "^^" + IRI__XSD_INTEGER));
    _writer->writeTriple(
        subj, IRI__OSMWAY_UNIQUE_NODE_COUNT,
        _writer->generateLiteralUnsafe(std::to_string(numUniquePoints),
                                       "^^" + IRI__XSD_INTEGER));
  }

  if (_config.addSortMetadata) {
    _writer->writeTriple(
        subj,
        _writer->generateIRIUnsafe(osm2rdf::ttl::constants::NAMESPACE__OSM_META,
                                   "length"),
        _writer->generateLiteral(
            std::to_string(boost::geometry::length(way.geom())),
            "^^" + osm2rdf::ttl::constants::IRI__XSD_DOUBLE));
  }
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
void osm2rdf::osm::FactHandler<W>::writeBoostGeometry(const std::string& subj,
                                                      const std::string& pred,
                                                      const G& geom) {
  std::ostringstream tmp;
  if (_config.simplifyWKT > 0 &&
      boost::geometry::num_points(geom) > _config.simplifyWKT) {
    G simplifiedGeom;
    auto perimeter_or_length = std::max(boost::geometry::perimeter(geom),
                                        boost::geometry::length(geom));
    do {
      boost::geometry::simplify(geom, simplifiedGeom,
                                BASE_SIMPLIFICATION_FACTOR *
                                    perimeter_or_length * _config.wktDeviation);
      perimeter_or_length /= 2;
    } while ((boost::geometry::is_empty(simplifiedGeom) ||
              !boost::geometry::is_valid(simplifiedGeom)) &&
             perimeter_or_length >= BASE_SIMPLIFICATION_FACTOR);
    tmp << std::fixed << std::setprecision(_config.wktPrecision)
        << boost::geometry::wkt(simplifiedGeom);
  } else {
    tmp << std::fixed << std::setprecision(_config.wktPrecision)
        << boost::geometry::wkt(geom);
  }
  _writer->writeTriple(subj, pred,
                       "\"" + tmp.str() + "\"^^" + IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeBox(const std::string& subj,
                                            const std::string& pred,
                                            const osm2rdf::geometry::Box& box) {
  // Box can not be simplified -> output directly.
  std::ostringstream tmp;
  tmp << std::fixed << std::setprecision(_config.wktPrecision)
      << boost::geometry::wkt(box);
  _writer->writeTriple(subj, pred,
                       "\"" + tmp.str() + "\"^^" + IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeTag(const std::string& subj,
                                            const osm2rdf::osm::Tag& tag) {
  const std::string& key = tag.first;
  const std::string& value = tag.second;
  if (key == "admin_level") {
    std::string objectValue;
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
      objectValue = _writer->generateLiteralUnsafe(std::to_string(lvl),
                                                   "^^" + IRI__XSD_INTEGER);
    } else {
      objectValue = _writer->generateLiteral(value, "");
    }

    _writer->writeTriple(subj, _writer->generateIRI(NAMESPACE__OSM_TAG, key),
                         objectValue);
  } else {
    try {
      _writer->writeTriple(subj, _writer->generateIRI(NAMESPACE__OSM_TAG, key),
                           _writer->generateLiteral(value, ""));
    } catch (const std::domain_error&) {
      const std::string& blankNode = _writer->generateBlankNode();
      _writer->writeTriple(subj, IRI__OSM_TAG, blankNode);
      _writer->writeTriple(blankNode,
                           _writer->generateIRI(NAMESPACE__OSM_TAG, "key"),
                           _writer->generateLiteral(key, ""));
      _writer->writeTriple(blankNode,
                           _writer->generateIRI(NAMESPACE__OSM_TAG, "value"),
                           _writer->generateLiteral(value, ""));
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
        const std::string& partialValue = value.substr(start, end);
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
      auto end = valueTmp.find(';');
      if (end != std::string::npos) {
        valueTmp = valueTmp.erase(end);
      }
      // Remove all but Q and digits to ensure Qdddddd format
      valueTmp.erase(
          remove_if(valueTmp.begin(), valueTmp.end(),
                    [](char chr) { return (chr != 'Q' && isdigit(chr) == 0); }),
          valueTmp.end());

      _writer->writeTriple(
          subj, _writer->generateIRI(NAMESPACE__OSM, key),
          _writer->generateIRI(NAMESPACE__WIKIDATA_ENTITY, valueTmp));
      tagTripleCount++;
    }
    if (!_config.skipWikiLinks &&
        (key == "wikipedia" || hasSuffix(key, ":wikipedia"))) {
      auto pos = value.find(':');
      if (pos != std::string::npos) {
        const std::string& lang = value.substr(0, pos);
        const std::string& entry = value.substr(pos + 1);
        _writer->writeTriple(
            subj, _writer->generateIRI(NAMESPACE__OSM, key),
            _writer->generateIRI("https://" + lang + ".wikipedia.org/wiki/",
                                 entry));
        tagTripleCount++;
      } else {
        _writer->writeTriple(
            subj, _writer->generateIRI(NAMESPACE__OSM, key),
            _writer->generateIRI("https://www.wikipedia.org/wiki/", value));
        tagTripleCount++;
      }
    }
  }
  _writer->writeTriple(
      subj, _writer->generateIRIUnsafe(NAMESPACE__OSM_META, "facts"),
      _writer->generateLiteralUnsafe(std::to_string(tagTripleCount),
                                     "^^" + IRI__XSD_INTEGER));
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
