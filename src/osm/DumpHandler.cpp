// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/DumpHandler.h"

#include <iomanip>
#include <iostream>

#include "boost/geometry.hpp"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/Way.h"
#include "osm2ttl/ttl/Writer.h"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::DumpHandler<W>::DumpHandler(const osm2ttl::config::Config& config,
                                          osm2ttl::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::area(const osm2ttl::osm::Area& area) {
  std::string s = _writer->generateIRI(
      area.fromWay() ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                     : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
      area.objId());

  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     area.geom());

  if (_config.addAreaEnvelope) {
    writeBox(s, osm2ttl::ttl::constants::IRI__OSM_ENVELOPE, area.envelope());
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::node(const osm2ttl::osm::Node& node) {
  std::string s = _writer->generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, node.id());

  _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
                       osm2ttl::ttl::constants::IRI__OSM_NODE);

  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     node.geom());

  writeTagList(s, node.tags());
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::relation(
    const osm2ttl::osm::Relation& relation) {
  std::string s = _writer->generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, relation.id());

  _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
                       osm2ttl::ttl::constants::IRI__OSM_RELATION);

  writeTagList(s, relation.tags());

  for (const auto& member : relation.members()) {
    const std::string& role = member.role();
    if (role != "outer" && role != "inner") {
      std::string node = _writer->generateBlankNode();
      _writer->writeTriple(
          s,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                               "member"),
          node);

      std::string type;
      switch (member.type()) {
        case osm2ttl::osm::RelationMemberType::NODE:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM_NODE;
          break;
        case osm2ttl::osm::RelationMemberType::RELATION:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION;
          break;
        case osm2ttl::osm::RelationMemberType::WAY:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM_WAY;
          break;
        default:
          type = osm2ttl::ttl::constants::NAMESPACE__OSM;
      }

      _writer->writeTriple(
          node,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "id"),
          _writer->generateIRI(type, member.id()));
      _writer->writeTriple(
          node,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, "role"),
          _writer->generateLiteral(role, ""));
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::way(const osm2ttl::osm::Way& way) {
  std::string s = _writer->generateIRI(
      osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, way.id());

  _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__RDF_TYPE,
                       osm2ttl::ttl::constants::IRI__OSM_WAY);

  writeTagList(s, way.tags());

  if (_config.addWayNodeOrder) {
    size_t i = 0;
    for (const auto& node : way.nodes()) {
      std::string blankNode = _writer->generateBlankNode();
      _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_NODE,
                           blankNode);

      _writer->writeTriple(
          blankNode, osm2ttl::ttl::constants::IRI__OSMWAY_NODE,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,
                               node.id()));

      _writer->writeTriple(
          blankNode, osm2ttl::ttl::constants::IRI__OSM_META__POS,
          _writer->generateLiteral(
              std::to_string(++i),
              "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
    }
  }

  osm2ttl::geometry::Linestring locations{way.geom()};
  size_t numUniquePoints = locations.size();
  writeBoostGeometry(s, osm2ttl::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,
                     locations);

  if (_config.addWayEnvelope) {
    writeBox(s, osm2ttl::ttl::constants::IRI__OSM_ENVELOPE, way.envelope());
  }

  if (_config.addWayMetaData) {
    _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_ISCLOSED,
                         way.closed() ? osm2ttl::ttl::constants::LITERAL__YES
                                      : osm2ttl::ttl::constants::LITERAL__NO);
    _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__OSMWAY_NODECOUNT,
                         _writer->generateLiteral(
                             std::to_string(way.nodes().size()),
                             "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
    _writer->writeTriple(s,
                         osm2ttl::ttl::constants::IRI__OSMWAY_UNIQUENODECOUNT,
                         _writer->generateLiteral(
                             std::to_string(numUniquePoints),
                             "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
  }
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
void osm2ttl::osm::DumpHandler<W>::writeBoostGeometry(const std::string& s,
                                                      const std::string& p,
                                                      const G& g) {
  const double onePercent = 0.01;
  G geom{g};
  if (_config.wktSimplify > 0 &&
      boost::geometry::num_points(g) > _config.wktSimplify) {
    osm2ttl::geometry::Box box;
    boost::geometry::envelope(geom, box);
    boost::geometry::simplify(
        g, geom,
        std::min(
            boost::geometry::get<boost::geometry::max_corner, 0>(box) -
                boost::geometry::get<boost::geometry::min_corner, 0>(box),
            boost::geometry::get<boost::geometry::max_corner, 1>(box) -
                boost::geometry::get<boost::geometry::min_corner, 1>(box)) /
            (onePercent * _config.wktDeviation));
    // If empty geometry -> use original
    if (!boost::geometry::is_valid(geom) || boost::geometry::is_empty(geom)) {
      geom = g;
    }
  }
  std::ostringstream tmp;
  tmp << std::fixed << std::setprecision(_config.wktPrecision)
      << boost::geometry::wkt(geom);
  _writer->writeTriple(
      s, p,
      "\"" + tmp.str() + "\"^^" +
          osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::writeBox(const std::string& s,
                                            const std::string& p,
                                            const osm2ttl::geometry::Box& box) {
  // Box can not be simplified -> output directly.
  std::ostringstream tmp;
  tmp << std::fixed << std::setprecision(_config.wktPrecision)
      << boost::geometry::wkt(box);
  _writer->writeTriple(
      s, p,
      "\"" + tmp.str() + "\"^^" +
          osm2ttl::ttl::constants::IRI__GEOSPARQL__WKT_LITERAL);
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::writeTag(const std::string& s,
                                            const osm2ttl::osm::Tag& tag) {
  const std::string& key = tag.first;
  const std::string& value = tag.second;
  if (key == "admin_level") {
    _writer->writeTriple(
        s,
        _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG, key),
        _writer->generateLiteral(
            value, "^^" + osm2ttl::ttl::constants::IRI__XSD_INTEGER));
  } else {
    try {
      _writer->writeTriple(
          s,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG,
                               key),
          _writer->generateLiteral(value, ""));
    } catch (const std::domain_error&) {
      std::string blankNode = _writer->generateBlankNode();
      _writer->writeTriple(s, osm2ttl::ttl::constants::IRI__OSM_TAG, blankNode);
      _writer->writeTriple(
          blankNode,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG,
                               "key"),
          _writer->generateLiteral(key, ""));
      _writer->writeTriple(
          blankNode,
          _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM_TAG,
                               "value"),
          _writer->generateLiteral(value, ""));
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::DumpHandler<W>::writeTagList(
    const std::string& s, const osm2ttl::osm::TagList& tags) {
  for (const auto& tag : tags) {
    writeTag(s, tag);
    const std::string& key = tag.first;
    std::string value = tag.second;
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
            _writer->generateIRI(osm2ttl::ttl::constants::NAMESPACE__OSM, key),
            _writer->generateIRI(
                osm2ttl::ttl::constants::NAMESPACE__WIKIDATA_ENTITY, value));
      }
      if (key == "wikipedia") {
        auto pos = value.find(':');
        if (pos != std::string::npos) {
          std::string lang = value.substr(0, pos);
          std::string entry = value.substr(pos + 1);
          _writer->writeTriple(
              s, osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA,
              _writer->generateIRI("https://" + lang + ".wikipedia.org/wiki/",
                                   entry));
        } else {
          _writer->writeTriple(
              s, osm2ttl::ttl::constants::IRI__OSM_WIKIPEDIA,
              _writer->generateIRI("https://www.wikipedia.org/wiki/", value));
        }
      }
    }
  }
}

// ____________________________________________________________________________
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::QLEVER>;
