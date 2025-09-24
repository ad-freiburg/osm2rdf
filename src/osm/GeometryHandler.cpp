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

#include "osm2rdf/osm/GeometryHandler.h"

#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/ttl/Constants.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/ProgressBar.h"
#include "osm2rdf/util/Time.h"
#include "spatialjoin/BoxIds.h"
#include "spatialjoin/Sweeper.h"
#include "spatialjoin/WKTParse.h"
#include "util/geo/Geo.h"

#if defined(_OPENMP)
#include "omp.h"
#endif

using osm2rdf::osm::Area;
using osm2rdf::osm::GeometryHandler;
using osm2rdf::osm::Relation;
using osm2rdf::osm::Way;

const static size_t BATCH_SIZE = 10000;

// ____________________________________________________________________________
template <typename W>
GeometryHandler<W>::GeometryHandler(const osm2rdf::config::Config& config,
                                    osm2rdf::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer),
      _sweeper(
          {static_cast<size_t>(config.numThreads),
           static_cast<size_t>(config.numThreads),
           300 * 1000 * 1000 * 5,
           10000,
           "",
           osm2rdf::ttl::constants::IRI__OPENGIS__INTERSECTS,
           osm2rdf::ttl::constants::IRI__OPENGIS__CONTAINS,
           osm2rdf::ttl::constants::IRI__OPENGIS__COVERS,
           osm2rdf::ttl::constants::IRI__OPENGIS__TOUCHES,
           osm2rdf::ttl::constants::IRI__OPENGIS__EQUALS,
           osm2rdf::ttl::constants::IRI__OPENGIS__OVERLAPS,
           osm2rdf::ttl::constants::IRI__OPENGIS__CROSSES,
           "\n",
           true,
           true,
           false,
           true,
           true,
           false,
           false,
           -1,
           false,
           [this](size_t t, const std::string& a, const std::string& b,
                  const std::string& pred) { this->writeRelCb(t, a, b, pred); },
           {},
           {},
           [this](size_t progr) { this->progressCb(progr); },
           {}},
          config.cache, ""),
      _parseBatches(config.numThreads) {}

// ___________________________________________________________________________
template <typename W>
GeometryHandler<W>::~GeometryHandler() = default;

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::relation(const Relation& rel) {
  if (rel.isArea()) return;  // skip area relations, will be handled by area()

  if (!rel.hasGeometry()) return;

  const std::string id = getSweeperId(rel.id(), 5);

  size_t subId = 0;

  if (rel.geom().size() > 1) subId = 1;

  for (const auto& m : rel.members()) {
    if (m.type() == osmium::item_type::node) {
      std::string pid = getSweeperId(m.positive_ref(), 1);
      _sweeper.add(pid, transform(::util::geo::getBoundingBox(rel.geom())), id,
                   subId, false, _parseBatches[omp_get_thread_num()]);
    }

    if (m.type() == osmium::item_type::way) {
      std::string pid = getSweeperId(m.positive_ref(), 2);
      _sweeper.add(pid, transform(::util::geo::getBoundingBox(rel.geom())), id,
                   subId, false, _parseBatches[omp_get_thread_num()]);
    }

    subId++;
  }

  if (_parseBatches[omp_get_thread_num()].size() > BATCH_SIZE) {
    _sweeper.addBatch(_parseBatches[omp_get_thread_num()]);
    _parseBatches[omp_get_thread_num()] = {};
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::writeRelCb(size_t t, const std::string& a,
                                    const std::string& b,
                                    const std::string& pred) {
  _writer->writeTriple(getFullID(a), pred, getFullID(b), t);
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::progressCb(size_t prog) {
  _progressBar.update(prog);
}

// ____________________________________________________________________________
template <typename W>
::util::geo::I32Box GeometryHandler<W>::transform(
    const ::util::geo::DBox& box) {
  return {transform(box.getLowerLeft()), transform(box.getUpperRight())};
}

// ____________________________________________________________________________
template <typename W>
::util::geo::I32Line GeometryHandler<W>::transform(
    const ::util::geo::DLine& way) {
  ::util::geo::I32Line l;

  l.reserve(way.size());

  for (const auto& loc : way) {
    l.push_back(transform(loc));
  }

  return l;
}

// ____________________________________________________________________________
template <typename W>
::util::geo::I32MultiPolygon GeometryHandler<W>::transform(
    const ::util::geo::DMultiPolygon& area) {
  ::util::geo::I32MultiPolygon p(area.size());

  for (size_t i = 0; i < area.size(); i++) {
    p[i].getOuter().reserve(area[i].getOuter().size());

    for (const auto& loc : area[i].getOuter()) {
      p[i].getOuter().push_back(transform(loc));
    }

    p[i].getInners().resize(area[i].getInners().size());

    for (size_t j = 0; j < area[i].getInners().size(); j++) {
      p[i].getInners()[j].reserve(area[i].getInners()[j].size());
      for (const auto& loc : area[i].getInners()[j]) {
        p[i].getInners()[j].push_back(transform(loc));
      }
    }
  }

  return p;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::area(const Area& area) {
  const std::string id = getSweeperId(area.objId(), area.fromWay() ? 2 : 4);

  _sweeper.add(transform(area.geom()), id, false,
               _parseBatches[omp_get_thread_num()]);

  if (_parseBatches[omp_get_thread_num()].size() > BATCH_SIZE) {
    _sweeper.addBatch(_parseBatches[omp_get_thread_num()]);
    _parseBatches[omp_get_thread_num()] = {};
  }
}

// ____________________________________________________________________________
template <typename W>
::util::geo::I32Point GeometryHandler<W>::transform(
    const ::util::geo::DPoint& loc) {
  auto point = ::util::geo::latLngToWebMerc(
      ::util::geo::DPoint(loc.getX(), loc.getY()));  // locs are lon/lat
  return ::util::geo::I32Point{static_cast<int>(point.getX() * PREC),
                               static_cast<int>(point.getY() * PREC)};
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::node(const osmium::Node& node) {
  if (!node.location().valid()) return;
  std::string id = getSweeperId(node.id(), 1);
  _sweeper.add(transform(::util::geo::DPoint{node.location().lon(),
                                             node.location().lat()}),
               id, false, _parseBatches[omp_get_thread_num()]);

  if (_parseBatches[omp_get_thread_num()].size() > BATCH_SIZE) {
    _sweeper.addBatch(_parseBatches[omp_get_thread_num()]);
    _parseBatches[omp_get_thread_num()] = {};
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::way(const Way& way) {
  if (way.isArea()) return;  // skip way relations, will be handled by area()

  std::string id = getSweeperId(way.id(), 2);

  _sweeper.add(transform(way.geom()), id, false,
               _parseBatches[omp_get_thread_num()]);

  if (_parseBatches[omp_get_thread_num()].size() > BATCH_SIZE) {
    _sweeper.addBatch(_parseBatches[omp_get_thread_num()]);
    _parseBatches[omp_get_thread_num()] = {};
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::calculateRelations() {
  // flush parse batches
  for (auto& b : _parseBatches) {
    _sweeper.addBatch(b);
    b = {};
  }

  // read optional auxiliary geo data
  for (const auto& auxFile : _config.auxGeoFiles) {
    if (auxFile.size() == 0) continue;
    const static size_t CACHE_SIZE = 1024 * 1024 * 100;
    unsigned char* buf = new unsigned char[CACHE_SIZE];

    auto file = open(auxFile.c_str(), O_RDONLY);

    if (file < 0) {
      delete[] buf;
      throw std::runtime_error("Could not read auxiliary geo file " + auxFile);
    }

    sj::WKTParser parser(&_sweeper, _config.numThreads);

    ssize_t len;

    while ((len = ::util::readAll(file, buf, CACHE_SIZE)) > 0) {
      parser.parse(reinterpret_cast<char*>(buf), len, 0);
    }

    parser.done();

    delete[] buf;
  }

  _sweeper.flush();

  _progressBar = osm2rdf::util::ProgressBar{_sweeper.numElements(), true};

  _sweeper.sweep();

  _progressBar.done();
}

// ____________________________________________________________________________
template <typename W>
std::string GeometryHandler<W>::getFullID(const std::string& strid) {
  uint64_t id = 0;

  for (size_t i = strid.size() - 1; i > 0; i--) {
    id |=
        static_cast<uint64_t>(reinterpret_cast<const unsigned char&>(strid[i]))
        << (8 * (strid.size() - 1 - i));
  }

  if (strid[0] == 1) {
    return _writer->generateIRI(
        osm2rdf::ttl::constants::NODE_NAMESPACE[_config.sourceDataset], id);
  }

  if (strid[0] == 2) {
    return _writer->generateIRI(
        osm2rdf::ttl::constants::WAY_NAMESPACE[_config.sourceDataset], id);
  }

  if (strid[0] == 4) {
    return _writer->generateIRI(
        osm2rdf::ttl::constants::RELATION_NAMESPACE[_config.sourceDataset], id);
  }

  if (strid[0] == 5) {
    return _writer->generateIRI(
        osm2rdf::ttl::constants::RELATION_NAMESPACE[_config.sourceDataset], id);
  }

  return strid;
}

// ____________________________________________________________________________
template <typename W>
std::string GeometryHandler<W>::getSweeperId(uint64_t oid, char type) {
  unsigned char id[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int a = 0;
  uint64_t tmp;

  while ((oid >> (a * 8))) {
    tmp = (oid & (0xFFLL << (a * 8)));
    id[8 - a] = tmp >> (a * 8);
    a++;
  }

  id[8 - a] = type;

  return std::string{reinterpret_cast<char*>(id + (8 - a)),
                     static_cast<size_t>(a + 1)};
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER>;
