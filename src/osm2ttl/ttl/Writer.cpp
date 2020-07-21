// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/Writer.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "boost/geometry.hpp"
#include "osmium/geom/factory.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/item_type.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/tag.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/geometry/Location.h"

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Box.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Tag.h"
#include "osm2ttl/osm/TagList.h"

#include "osm2ttl/ttl/BlankNode.h"
#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/Literal.h"

// ____________________________________________________________________________
osm2ttl::ttl::Writer::Writer(const osm2ttl::config::Config& config)
  : _config(config),
  _outQueue(_config.numThreadsWrite, _config.queueFactorWrite, "Writer::out"),
  _convertQueue(_config.numThreadsConvertGeom, _config.queueFactorConvertGeom,
                "Writer::geom") {
  _out = &std::cout;
}

// ____________________________________________________________________________
osm2ttl::ttl::Writer::~Writer() {
  close();
}

// ____________________________________________________________________________
bool osm2ttl::ttl::Writer::open() {
  if (!_config.output.empty()) {
    _outFile.open(_config.output);
    _out = &_outFile;
    return _outFile.is_open();
  }
  return true;
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::close() {
  _convertQueue.quit();
  _outQueue.quit();
  if (_outFile.is_open()) {
    _outFile.close();
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::writeHeader() {
  const std::lock_guard<std::mutex> lock(_outMutex);
  *_out << _config.outputFormat.header();
}

// ____________________________________________________________________________
template<typename S, typename O>
void osm2ttl::ttl::Writer::writeTriple(const S& s, const osm2ttl::ttl::IRI& p,
                                     const O& o) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  static_assert(std::is_same<O, osm2ttl::ttl::BlankNode>::value
                || std::is_same<O, osm2ttl::ttl::IRI>::value
                || std::is_same<O, osm2ttl::ttl::Literal>::value);
  auto f = [this, s, p, o]{
    std::string line = _config.outputFormat.format(s) + " " \
      + _config.outputFormat.format(p) + " " \
      + _config.outputFormat.format(o) + " .\n";
    const std::lock_guard<std::mutex> lock(_outMutex);
    *_out << std::move(line);
  };
  if (_config.numThreadsWrite > 0) {
    _outQueue.dispatch(f);
  } else {
    f();
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::osm::Area& area) {
  osm2ttl::ttl::IRI s{area.fromWay()?"osmway":"osmrel",
      std::to_string(area.objId())};

  writeBoostGeometry(s, osm2ttl::ttl::IRI("geo", "hasGeometry"), area.geom());

  if (_config.addEnvelope) {
    writeBox(s, osm2ttl::ttl::IRI("osm", "envelope"), area.envelope());
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::osm::Node& node) {
  osm2ttl::ttl::IRI s{"osmnode", node};

  writeTriple(s,
    osm2ttl::ttl::IRI("rdf", "type"),
    osm2ttl::ttl::IRI("osm", "node"));

  writeBoostGeometry(s, osm2ttl::ttl::IRI("geo", "hasGeometry"), node.geom());

  writeTagList(s, node.tags());
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::osm::Relation& relation) {
  osm2ttl::ttl::IRI s{"osmrel", relation};

  writeTriple(s,
    osm2ttl::ttl::IRI("rdf", "type"),
    osm2ttl::ttl::IRI("osm", "relation"));

  writeTagList(s, relation.tags());

  for (const auto& member : relation.members()) {
    const std::string& role = member.role();
    if (role != "outer" && role != "inner") {
      std::string type = "osm";
      if (member.type() == osm2ttl::osm::RelationMemberType::NODE) {
        type = "osmnode";
      } else if (member.type() == osm2ttl::osm::RelationMemberType::RELATION) {
        type = "osmrel";
      } else if (member.type() == osm2ttl::osm::RelationMemberType::WAY) {
        type = "osmway";
      }
      writeTriple(s,
        osm2ttl::ttl::IRI("osmrel", role),
        osm2ttl::ttl::IRI(type, member));
    }
  }
}

// ____________________________________________________________________________
void osm2ttl::ttl::Writer::write(const osm2ttl::osm::Way& way) {
  osm2ttl::ttl::IRI s{"osmway", way};

  writeTriple(s,
    osm2ttl::ttl::IRI("rdf", "type"),
    osm2ttl::ttl::IRI("osm", "way"));

  writeTagList(s, way.tags());

  if (_config.expandedData) {
    size_t i = 0;
    for (const auto& node : way.nodes()) {
      osm2ttl::ttl::BlankNode b;
      writeTriple(s, osm2ttl::ttl::IRI("osmway", "node"), b);

      writeTriple(b,
        osm2ttl::ttl::IRI("osmway", "node"),
        osm2ttl::ttl::IRI("osmnode", node));

      writeTriple(b,
        osm2ttl::ttl::IRI("osmm", "pos"),
        osm2ttl::ttl::Literal(std::to_string(++i),
          osm2ttl::ttl::IRI("xsd", "integer")));
    }
  }

  osm2ttl::geometry::Linestring locations{way.geom()};
  size_t numUniquePoints = locations.size();
  writeBoostGeometry(s, osm2ttl::ttl::IRI("geo", "hasGeometry"), locations);

  if (_config.metaData) {
    writeTriple(s,
      osm2ttl::ttl::IRI("osmway", "is_closed"),
      osm2ttl::ttl::Literal(way.closed()?"yes":"no"));
    writeTriple(s,
      osm2ttl::ttl::IRI("osmway", "nodeCount"),
      osm2ttl::ttl::Literal(std::to_string(way.nodes().size())));
    writeTriple(s,
      osm2ttl::ttl::IRI("osmway", "uniqueNodeCount"),
      osm2ttl::ttl::Literal(std::to_string(numUniquePoints)));
  }

  if (_config.addEnvelope) {
    writeBox(s, osm2ttl::ttl::IRI("osm", "envelope"), way.envelope());
  }
}

// ____________________________________________________________________________
template<typename S, typename G>
void osm2ttl::ttl::Writer::writeBoostGeometry(const S&s,
                                              const osm2ttl::ttl::IRI& p,
                                              const G& g) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  auto f = [this, s, p, g]{
    const double onePercent = 0.01;
    G geom{g};
    if (_config.wktSimplify && boost::geometry::num_points(g) > 4) {
      osm2ttl::geometry::Box box;
      boost::geometry::envelope(geom, box);
      boost::geometry::simplify(g, geom,
        std::min(boost::geometry::get<boost::geometry::max_corner, 0>(box)
               - boost::geometry::get<boost::geometry::min_corner, 0>(box),
                 boost::geometry::get<boost::geometry::max_corner, 1>(box)
               - boost::geometry::get<boost::geometry::min_corner, 1>(box))
        / (onePercent * _config.wktDeviation));
      // If empty geometry -> use original
      if (!boost::geometry::is_valid(geom) || boost::geometry::is_empty(geom)) {
        geom = g;
      }
    }
    std::ostringstream tmp;
    tmp << std::setprecision(_config.wktPrecision)
      << boost::geometry::wkt(geom);
    writeTriple(s, p, osm2ttl::ttl::Literal(tmp.str()));
  };
  if (_config.numThreadsConvertGeom > 0) {
    _outQueue.dispatch(f);
  } else {
    f();
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeBox(const S& s,
                                    const osm2ttl::ttl::IRI& p,
                                    const osm2ttl::osm::Box& box) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  auto f = [this, s, p, box]{
    // Box can not be simplified -> output directly.
    std::ostringstream tmp;
    tmp << boost::geometry::wkt(box.geom());
    writeTriple(s, p, osm2ttl::ttl::Literal(tmp.str()));
  };
  if (_config.numThreadsConvertGeom > 0) {
    _outQueue.dispatch(f);
  } else {
    f();
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeTag(const S& s, const osm2ttl::osm::Tag& tag) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  const std::string &key = tag.first;
  const std::string &value = tag.second;
  auto tagType = _config.tagKeyType.find(key);
  if (tagType != _config.tagKeyType.end()) {
    writeTriple(s,
      osm2ttl::ttl::IRI("osmt", key),
      osm2ttl::ttl::Literal(value, tagType->second));
  } else {
    writeTriple(s,
      osm2ttl::ttl::IRI("osmt", key),
      osm2ttl::ttl::Literal(value));
  }
}

// ____________________________________________________________________________
template<typename S>
void osm2ttl::ttl::Writer::writeTagList(const S& s,
                                        const osm2ttl::osm::TagList& tags) {
  static_assert(std::is_same<S, osm2ttl::ttl::BlankNode>::value
                || std::is_same<S, osm2ttl::ttl::IRI>::value);
  for (const osm2ttl::osm::Tag& tag : tags) {
    writeTag(s, tag);
    const std::string &key = tag.first;
    std::string value = tag.second;
    if (!_config.skipWikiLinks) {
      if (key == "wikidata") {
        // Only take first wikidata entry if ; is found
        auto end = value.find(';');
        if (end != std::string::npos) {
          value = value.erase(end);
        }
        // Remove all but Q and digits to ensuder Qdddddd format
        value.erase(remove_if(value.begin(), value.end(), [](char c) {
          return (!isdigit(c) && c != 'Q');
        }), value.end());

        writeTriple(s,
          osm2ttl::ttl::IRI("osm", key),
          osm2ttl::ttl::IRI("wd", value));
      }
      if (key == "wikipedia") {
        auto pos = value.find(':');
        if (pos != std::string::npos) {
          std::string lang = value.substr(0, pos);
          std::string entry = value.substr(pos + 1);
          writeTriple(s,
            osm2ttl::ttl::IRI("osm", "wikipedia"),
            osm2ttl::ttl::IRI("https://"+lang+".wikipedia.org/wiki/", entry));
        } else {
          writeTriple(s,
            osm2ttl::ttl::IRI("osm", "wikipedia"),
            osm2ttl::ttl::IRI("https://www.wikipedia.org/wiki/", value));
        }
      }
    }
  }
}
