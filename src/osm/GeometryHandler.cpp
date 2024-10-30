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
#include "osm2rdf/osm/GeometryHandler.h"
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
using osm2rdf::osm::Node;
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
           "",
           osm2rdf::ttl::constants::IRI__OPENGIS_INTERSECTS,
           osm2rdf::ttl::constants::IRI__OPENGIS_CONTAINS,
           osm2rdf::ttl::constants::IRI__OPENGIS_COVERS,
           osm2rdf::ttl::constants::IRI__OPENGIS_TOUCHES,
           osm2rdf::ttl::constants::IRI__OPENGIS_EQUALS,
           osm2rdf::ttl::constants::IRI__OPENGIS_OVERLAPS,
           osm2rdf::ttl::constants::IRI__OPENGIS_CROSSES,
           "\n",
           true,
           true,
           true,
           true,
           true,
           true,
           false,
           false,
           [this](size_t t, const std::string& a, const std::string& b,
                  const std::string& pred) { this->writeRelCb(t, a, b, pred); },
           {},
           {},
           [this](size_t progr) { this->progressCb(progr); }},
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

  const std::string id = _writer->generateIRI(
      osm2rdf::ttl::constants::RELATION_NAMESPACE[_config.sourceDataset],
      rel.id());

  size_t subId = 0;

  if (rel.geom().size() > 1) subId = 1;

  for (const auto& m : rel.members()) {
    if (m.type() == osm2rdf::osm::RelationMemberType::NODE) {
      std::string pid = _writer->generateIRI(
          osm2rdf::ttl::constants::NODE_NAMESPACE[_config.sourceDataset],
          m.id());
      _sweeper.add(pid, transform(rel.envelope()), id, subId, false,
                   _parseBatches[omp_get_thread_num()]);
    }

    if (m.type() == osm2rdf::osm::RelationMemberType::WAY) {
      std::string pid = _writer->generateIRI(
          osm2rdf::ttl::constants::WAY_NAMESPACE[_config.sourceDataset],
          m.id());
      _sweeper.add(pid, transform(rel.envelope()), id, subId, false,
                   _parseBatches[omp_get_thread_num()]);
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
  _writer->writeTriple(a, pred, b, t);
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
  const std::string id = _writer->generateIRI(
      areaNS(area.fromWay() ? AreaFromType::WAY : AreaFromType::RELATION),
      area.objId());

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
void GeometryHandler<W>::node(const Node& node) {
  std::string id = _writer->generateIRI(
      osm2rdf::ttl::constants::NODE_NAMESPACE[_config.sourceDataset],
      node.id());

  _sweeper.add(transform(node.geom()), id, false,
               _parseBatches[omp_get_thread_num()]);

  if (_parseBatches[omp_get_thread_num()].size() > BATCH_SIZE) {
    _sweeper.addBatch(_parseBatches[omp_get_thread_num()]);
    _parseBatches[omp_get_thread_num()] = {};
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::way(const Way& way) {
  if (way.isArea()) return;  // skip way relations, will be handled by area()

  std::string id = _writer->generateIRI(
      osm2rdf::ttl::constants::WAY_NAMESPACE[_config.sourceDataset], way.id());

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

    ::util::JobQueue<ParseBatch> jobs(1000);             // the WKT parse jobs
    std::vector<std::thread> thrds(_config.numThreads);  // the parse workers
    for (size_t i = 0; i < thrds.size(); i++)
      thrds[i] = std::thread(&processQueue, &jobs, i, &_sweeper);

    ssize_t len;
    std::string dangling;
    size_t gid = 0;

    while ((len = ::util::readAll(file, buf, CACHE_SIZE)) > 0) {
      parse(reinterpret_cast<char*>(buf), len, dangling, &gid, jobs, 0);
    }

    // end event
    jobs.add({});

    // wait for all parse workers to finish
    for (auto& thr : thrds) thr.join();

    delete[] buf;
  }

  _sweeper.flush();

  _progressBar = osm2rdf::util::ProgressBar{_sweeper.numElements(), true};

  _sweeper.sweep();

  _progressBar.done();
}

// ____________________________________________________________________________
template <typename W>
std::string GeometryHandler<W>::areaNS(AreaFromType type) const {
  switch (type) {
    case AreaFromType::RELATION:
      return osm2rdf::ttl::constants::RELATION_NAMESPACE[_config.sourceDataset];
    case AreaFromType::WAY:
      return osm2rdf::ttl::constants::WAY_NAMESPACE[_config.sourceDataset];
    default:
      return osm2rdf::ttl::constants::WAY_NAMESPACE[_config.sourceDataset];
  }
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::TTL>;
