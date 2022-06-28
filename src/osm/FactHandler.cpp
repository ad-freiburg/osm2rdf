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

#include "osm2rdf/osm/FactHandler.h"

#include <iomanip>
#include <iostream>

#include "boost/geometry.hpp"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/geometry/Location.h"
#include "osm2rdf/osm/Node.h"
#include "osm2rdf/osm/Relation.h"
#include "osm2rdf/osm/Way.h"
#include "osm2rdf/ttl/Writer.h"

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::FactHandler<W>::FactHandler(const osm2rdf::config::Config& config,
                                          osm2rdf::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::area(const osm2rdf::osm::Area& area) {
  std::string s = _writer->generateIRI(
      area.fromWay() ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                     : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
      area.objId());

  writeBoostGeometry(s, osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     area.geom());

  if (_config.addAreaEnvelope) {
    writeBox(s, osm2rdf::ttl::constants::IRI__OSM_META__ENVELOPE,
             area.envelope());
  }

  if (_config.addSortMetadata) {
    std::ostringstream tmp;
    // Increase default precision as areas in regbez freiburg have a 0 area
    // otherwise.
    tmp << std::fixed
        << std::setprecision(osm2rdf::osm::constants::AREA_PRECISION)
        << area.geomArea();
    _writer->writeTriple(
        s,
        _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_META,
                             "area"),
        _writer->generateLiteral(
            tmp.str(), "^^" + osm2rdf::ttl::constants::IRI__XSD_DOUBLE));
  }

  if (_config.addAreaEnvelopeRatio) {
    std::ostringstream tmp;
    tmp << std::fixed << (area.geomArea() / area.envelopeArea());
    _writer->writeTriple(
        s,
        _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_META,
                             "area_envelope_ratio"),
        _writer->generateLiteral(
            tmp.str(), "^^" + osm2rdf::ttl::constants::IRI__XSD_DOUBLE));
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::node(const osm2rdf::osm::Node& node) {
  std::string s = _writer->generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, node.id());

  _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__RDF_TYPE,
                       osm2rdf::ttl::constants::IRI__OSM_NODE);

  writeBoostGeometry(s, osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     node.geom());

  writeTagList(s, node.tags());

  if (_config.addNodeEnvelope) {
    writeBox(s, osm2rdf::ttl::constants::IRI__OSM_META__ENVELOPE,
             node.envelope());
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::relation(
    const osm2rdf::osm::Relation& relation) {
  std::string s = _writer->generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION, relation.id());

  _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__RDF_TYPE,
                       osm2rdf::ttl::constants::IRI__OSM_RELATION);

  writeTagList(s, relation.tags());

  size_t i = 0;
  for (const auto& member : relation.members()) {
    const std::string& role = member.role();
    if (_config.addRelationBorderMembers ||
        (role != "outer" && role != "inner")) {
      std::string blankNode = _writer->generateBlankNode();
      _writer->writeTriple(
          s,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
                               "member"),
          blankNode);

      std::string type;
      switch (member.type()) {
        case osm2rdf::osm::RelationMemberType::NODE:
          type = osm2rdf::ttl::constants::NAMESPACE__OSM_NODE;
          break;
        case osm2rdf::osm::RelationMemberType::RELATION:
          type = osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION;
          break;
        case osm2rdf::osm::RelationMemberType::WAY:
          type = osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
          break;
        default:
          type = osm2rdf::ttl::constants::NAMESPACE__OSM;
      }

      _writer->writeTriple(
          blankNode,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, "id"),
          _writer->generateIRI(type, member.id()));
      _writer->writeTriple(
          blankNode,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, "role"),
          _writer->generateLiteral(role, ""));
      _writer->writeTriple(
          blankNode, osm2rdf::ttl::constants::IRI__OSM_META__POS,
          _writer->generateLiteral(
              std::to_string(i++),
              "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER));
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::way(const osm2rdf::osm::Way& way) {
  std::string s = _writer->generateIRI(
      osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, way.id());

  _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__RDF_TYPE,
                       osm2rdf::ttl::constants::IRI__OSM_WAY);

  writeTagList(s, way.tags());

  if (_config.addWayNodeOrder) {
    size_t i = 0;
    std::string lastBlankNode;
    auto lastNode = way.nodes().front();
    for (const auto& node : way.nodes()) {
      std::string blankNode = _writer->generateBlankNode();
      _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__OSMWAY_NODE,
                           blankNode);

      _writer->writeTriple(
          blankNode, osm2rdf::ttl::constants::IRI__OSMWAY_NODE,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE,
                               node.id()));

      _writer->writeTriple(
          blankNode, osm2rdf::ttl::constants::IRI__OSM_META__POS,
          _writer->generateLiteral(
              std::to_string(i++),
              "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER));

      if (_config.addWayNodeGeometry) {
        std::string s = _writer->generateIRI(
            osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, node.id());

        _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__RDF_TYPE,
                             osm2rdf::ttl::constants::IRI__OSM_NODE);

        writeBoostGeometry(
            s, osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
            node.geom());
      }

      if (_config.addWayNodeSpatialMetadata && !lastBlankNode.empty()) {
        _writer->writeTriple(
            lastBlankNode, osm2rdf::ttl::constants::IRI__OSMWAY_NEXT_NODE,
            _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_NODE,
                                 node.id()));
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
            lastBlankNode,
            osm2rdf::ttl::constants::IRI__OSMWAY_NEXT_NODE_DISTANCE,
            _writer->generateLiteral(
                std::to_string(distance),
                "^^" + osm2rdf::ttl::constants::IRI__XSD_DECIMAL));
      }
      lastBlankNode = blankNode;
      lastNode = node;
    }
  }

  osm2rdf::geometry::Linestring locations{way.geom()};
  size_t numUniquePoints = locations.size();
  writeBoostGeometry(s, osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     locations);

  if (_config.addWayEnvelope) {
    writeBox(s, osm2rdf::ttl::constants::IRI__OSM_META__ENVELOPE,
             way.envelope());
  }

  if (_config.addWayMetadata) {
    _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__OSMWAY_IS_CLOSED,
                         way.closed() ? osm2rdf::ttl::constants::LITERAL__YES
                                      : osm2rdf::ttl::constants::LITERAL__NO);
    _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__OSMWAY_NODE_COUNT,
                         _writer->generateLiteral(
                             std::to_string(way.nodes().size()),
                             "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER));
    _writer->writeTriple(s,
                         osm2rdf::ttl::constants::IRI__OSMWAY_UNIQUE_NODE_COUNT,
                         _writer->generateLiteral(
                             std::to_string(numUniquePoints),
                             "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER));
  }

  if (_config.addSortMetadata) {
    _writer->writeTriple(
        s,
        _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_META,
                             "length"),
        _writer->generateLiteral(
            std::to_string(boost::geometry::length(way.geom())),
            "^^" + osm2rdf::ttl::constants::IRI__XSD_DOUBLE));
  }
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
void osm2rdf::osm::FactHandler<W>::writeBoostGeometry(const std::string& s,
                                                      const std::string& p,
                                                      const G& g) {
  std::ostringstream tmp;

  if (_config.simplifyWKT > 0 &&
      boost::geometry::num_points(g) > _config.simplifyWKT) {
    G geom;
    auto perimeter_or_length = std::max(boost::geometry::perimeter(g),
                                        boost::geometry::length(g));
    do {
      boost::geometry::simplify(
          g, geom,
          osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR *
              perimeter_or_length * _config.wktDeviation);
      perimeter_or_length /= 2;
    } while (
        (boost::geometry::is_empty(geom) || !boost::geometry::is_valid(geom)) &&
        perimeter_or_length >=
            osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR);

    auto scaledOut = xyToLatLng(geom);
    tmp << std::fixed << std::setprecision(_config.wktPrecision)
        << boost::geometry::wkt(scaledOut);
  } else {
    auto scaledOut = xyToLatLng(g);
    tmp << std::fixed << std::setprecision(_config.wktPrecision)
        << boost::geometry::wkt(scaledOut);
  }

  _writer->writeTriple(
      s, p,
      "\"" + tmp.str() + "\"^^" +
          osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeBox(const std::string& s,
                                            const std::string& p,
                                            const osm2rdf::geometry::Box& box) {
  // Box can not be simplified -> output directly.
  boost::geometry::model::box<boost::geometry::model::d2::point_xy<double>>
      boxScaled;

  boost::geometry::strategy::transform::scale_transformer<double, 2, 2> scale(
      1.0 / 10000000.0);
  boost::geometry::transform(box, boxScaled, scale);

  std::ostringstream tmp;
  tmp << std::fixed << std::setprecision(_config.wktPrecision)
      << boost::geometry::wkt(boxScaled);
  _writer->writeTriple(
      s, p,
      "\"" + tmp.str() + "\"^^" +
          osm2rdf::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeTag(const std::string& s,
                                            const osm2rdf::osm::Tag& tag) {
  const std::string& key = tag.first;
  const std::string& value = tag.second;
  if (key == "admin_level") {
    _writer->writeTriple(
        s,
        _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG, key),
        _writer->generateLiteral(
            value, "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER));
  } else {
    try {
      _writer->writeTriple(
          s,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG,
                               key),
          _writer->generateLiteral(value, ""));
    } catch (const std::domain_error&) {
      std::string blankNode = _writer->generateBlankNode();
      _writer->writeTriple(s, osm2rdf::ttl::constants::IRI__OSM_TAG, blankNode);
      _writer->writeTriple(
          blankNode,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG,
                               "key"),
          _writer->generateLiteral(key, ""));
      _writer->writeTriple(
          blankNode,
          _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_TAG,
                               "value"),
          _writer->generateLiteral(value, ""));
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::FactHandler<W>::writeTagList(
    const std::string& s, const osm2rdf::osm::TagList& tags) {
  size_t tagTripleCount = 0;
  for (const auto& tag : tags) {
    const std::string& key = tag.first;
    std::string value = tag.second;
    // Special handling for ref tag splitting. Maybe generalize this...
    if (_config.semicolonTagKeys.find(key) != _config.semicolonTagKeys.end() &&
        value.find(';') != std::string::npos) {
      size_t end;
      size_t start = 0;
      while ((end = value.find(';', start)) != std::string::npos) {
        std::string partialValue = value.substr(start, end);
        writeTag(s, osm2rdf::osm::Tag(key, partialValue));
        tagTripleCount++;
        start = end + 1;
      };
      std::string partialValue = value.substr(start, value.size());
      writeTag(s, osm2rdf::osm::Tag(key, partialValue));
      tagTripleCount++;
    } else {
      writeTag(s, tag);
      tagTripleCount++;
    }
    // Handling for wiki tags
    if (!_config.skipWikiLinks) {
      if (key == "wikidata") {
        // Only take first wikidata entry if ; is found
        auto end = value.find(';');
        if (end != std::string::npos) {
          value = value.erase(end);
        }
        // Remove all but Q and digits to ensure Qdddddd format
        value.erase(
            remove_if(value.begin(), value.end(),
                      [](char c) { return (c != 'Q' && isdigit(c) == 0); }),
            value.end());

        _writer->writeTriple(
            s,
            _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM, key),
            _writer->generateIRI(
                osm2rdf::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, value));
        tagTripleCount++;
      }
      if (key == "wikipedia") {
        auto pos = value.find(':');
        if (pos != std::string::npos) {
          std::string lang = value.substr(0, pos);
          std::string entry = value.substr(pos + 1);
          _writer->writeTriple(
              s, osm2rdf::ttl::constants::IRI__OSM_WIKIPEDIA,
              _writer->generateIRI("https://" + lang + ".wikipedia.org/wiki/",
                                   entry));
          tagTripleCount++;
        } else {
          _writer->writeTriple(
              s, osm2rdf::ttl::constants::IRI__OSM_WIKIPEDIA,
              _writer->generateIRI("https://www.wikipedia.org/wiki/", value));
          tagTripleCount++;
        }
      }
    }
  }
  _writer->writeTriple(
      s,
      _writer->generateIRI(osm2rdf::ttl::constants::NAMESPACE__OSM_META,
                           "facts"),
      _writer->generateLiteral(
          std::to_string(tagTripleCount),
          "^^" + osm2rdf::ttl::constants::IRI__XSD_INTEGER));
}

// ____________________________________________________________________________
template <typename W>
boost::geometry::model::d2::point_xy<double>
osm2rdf::osm::FactHandler<W>::xyToLatLng(const osm2rdf::geometry::Location& l) {
  boost::geometry::model::d2::point_xy<double> gScaled;

  boost::geometry::strategy::transform::scale_transformer<double, 2, 2> scale(
      1.0 / 10000000.0);
  boost::geometry::transform(l, gScaled, scale);

  return gScaled;
}

// ____________________________________________________________________________
template <typename W>
boost::geometry::model::linestring<boost::geometry::model::d2::point_xy<double>>
osm2rdf::osm::FactHandler<W>::xyToLatLng(
    const osm2rdf::geometry::Linestring& l) {
  boost::geometry::model::linestring<
      boost::geometry::model::d2::point_xy<double>>
      gScaled;

  boost::geometry::strategy::transform::scale_transformer<double, 2, 2> scale(
      1.0 / 10000000.0);
  boost::geometry::transform(l, gScaled, scale);

  return gScaled;
}

// ____________________________________________________________________________
template <typename W>
boost::geometry::model::multi_polygon<
    boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>>>
osm2rdf::osm::FactHandler<W>::xyToLatLng(const osm2rdf::geometry::Area& l) {
  boost::geometry::model::multi_polygon<
      boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>>>
      gScaled;

  boost::geometry::strategy::transform::scale_transformer<double, 2, 2> scale(
      1.0 / 10000000.0);
  boost::geometry::transform(l, gScaled, scale);

  return gScaled;
}

// ____________________________________________________________________________
template class osm2rdf::osm::FactHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::FactHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::FactHandler<osm2rdf::ttl::format::QLEVER>;
