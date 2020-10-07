// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "boost/geometry.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/thread.hpp"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Constants.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/util/progress_bar.hpp"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::GeometryHandler<W>::GeometryHandler(
    const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::area(const osmium::Area& area) {
  if (_config.noContains) {
    return;
  }
  if (_config.noAreaDump) {
    return;
  }
  osm2ttl::osm::Area a = osm2ttl::osm::Area(area);
  if (!a.hasName()) {
    return;
  }
  _spatialStorageArea.emplace_back(a.envelope(), a.objId(), a.geom(),
                                   a.fromWay());
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::node(const osmium::Node& node) {
  if (_config.noContains) {
    return;
  }
  if (_config.noNodeDump) {
    return;
  }
  osm2ttl::osm::Node n = osm2ttl::osm::Node(node);
  if (n.tags().empty()) {
    return;
  }
  _spatialStorageNode.emplace_back(n.envelope(), n.id(), n.geom());
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::way(const osmium::Way& way) {
  if (_config.noContains) {
    return;
  }
  if (_config.noWayDump) {
    return;
  }
  osm2ttl::osm::Way w = osm2ttl::osm::Way(way);
  if (w.tags().empty()) {
    return;
  }
  _spatialStorageWay.emplace_back(w.envelope(), w.id(), w.geom());
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::lookup() {
  if (_config.noContains) {
    return;
  }
  SpatialIndex spatialIndex;
  {
    std::cerr << " Packing combined tree with " << _spatialStorageArea.size()
              << " entries ... " << std::flush;
    spatialIndex =
        SpatialIndex(_spatialStorageArea.begin(), _spatialStorageArea.end());
    std::cerr << "done" << std::endl;
  }

  if (!_config.noAreaDump) {
    std::cerr << " Contains relations for " << _spatialStorageArea.size()
              << " areas in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;
    progressBar.update(entryCount);

#pragma omp parallel shared(                          \
    entryCount, progressBar, spatialIndex, std::cerr, \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,       \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,   \
    osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,     \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
    {
#pragma omp single
      {
        for (auto& entry : _spatialStorageArea) {
          auto entryEnvelope = std::get<0>(entry);
          auto entryId = std::get<1>(entry);
          auto entryGeom = std::get<2>(entry);
          std::string entryIRI = _writer->generateIRI(
              std::get<3>(entry)
                  ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                  : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
              entryId);
          for (auto it = spatialIndex.qbegin(
                   boost::geometry::index::covers(entryEnvelope));
               it != spatialIndex.qend(); it++) {
            auto areaId = std::get<1>(*it);
            auto areaGeom = std::get<2>(*it);
            auto areaFromWay = std::get<3>(*it);
#pragma omp task firstprivate(areaFromWay, areaGeom, areaId, entryGeom, \
                              entryIRI, entryId)                        \
    shared(osm2ttl::ttl::constants::IRI__OGC_CONTAINS, std::cerr,       \
           osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,              \
           osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,                \
           osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,            \
           osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
            {
              if (areaId != entryId &&
                  boost::geometry::covered_by(entryGeom, areaGeom)) {
                std::string areaIRI = _writer->generateIRI(
                    areaFromWay
                        ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                    areaId);
                _writer->writeTriple(areaIRI,
                                     osm2ttl::ttl::constants::IRI__OGC_CONTAINS,
                                     entryIRI);
                _writer->writeTriple(
                    entryIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                    areaIRI);
              }
            }
          }
          progressBar.update(entryCount++);
        }
      }
    }
    progressBar.done();
    std::cerr << " ... done" << std::endl;
  }

  if (!_config.noNodeDump) {
    std::cerr << " Contains relations for " << _spatialStorageNode.size()
              << " nodes in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageNode.size(), true};
    size_t entryCount = 0;
    progressBar.update(entryCount);

#pragma omp parallel shared(                          \
    entryCount, progressBar, spatialIndex,            \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,       \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,   \
    osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,     \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
    {
#pragma omp single
      {
        for (auto& node : _spatialStorageNode) {
          auto nodeEnvelope = std::get<0>(node);
          auto nodeId = std::get<1>(node);
          auto nodeGeom = std::get<2>(node);
          std::string nodeIRI = _writer->generateIRI(
              osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, nodeId);
          for (auto it = spatialIndex.qbegin(
                   boost::geometry::index::covers(nodeEnvelope));
               it != spatialIndex.qend(); it++) {
            auto areaId = std::get<1>(*it);
            auto areaGeom = std::get<2>(*it);
            auto areaFromWay = std::get<3>(*it);
#pragma omp task firstprivate(areaFromWay, areaGeom, areaId, nodeGeom, \
                              nodeIRI)                                 \
    shared(osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                 \
           osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,             \
           osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,               \
           osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,           \
           osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
            {
              if (boost::geometry::covered_by(nodeGeom, areaGeom)) {
                std::string areaIRI = _writer->generateIRI(
                    areaFromWay
                        ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                    areaId);
                _writer->writeTriple(areaIRI,
                                     osm2ttl::ttl::constants::IRI__OGC_CONTAINS,
                                     nodeIRI);
                _writer->writeTriple(
                    nodeIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                    areaIRI);
              }
            }
          }
          progressBar.update(entryCount++);
        }
      }
    }
    progressBar.done();
    std::cerr << " ... done" << std::endl;
  }

  if (!_config.noWayDump) {
    std::cerr << " Contains relations for " << _spatialStorageWay.size()
              << " ways in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageWay.size(), true};
    size_t entryCount = 0;
    progressBar.update(entryCount);

#pragma omp parallel shared(                          \
    entryCount, progressBar, spatialIndex, std::cerr, \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,       \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,   \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
    {
#pragma omp single
      {
        for (auto& way : _spatialStorageWay) {
          auto wayEnvelope = std::get<0>(way);
          auto wayId = std::get<1>(way);
          auto wayGeom = std::get<2>(way);
          std::string wayIRI = _writer->generateIRI(
              osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, wayId);
          for (auto it = spatialIndex.qbegin(
                   boost::geometry::index::covers(wayEnvelope));
               it != spatialIndex.qend(); it++) {
            auto areaId = std::get<1>(*it);
            auto areaGeom = std::get<2>(*it);
            auto areaFromWay = std::get<3>(*it);
#pragma omp task firstprivate(areaFromWay, areaGeom, areaId, wayGeom, wayId, \
                              wayIRI)                                        \
    shared(osm2ttl::ttl::constants::IRI__OGC_CONTAINS, std::cerr,            \
           osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,                   \
           osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                 \
           osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
            {
              if ((!areaFromWay || areaId != wayId) &&
                  boost::geometry::covered_by(wayGeom, areaGeom)) {
                std::string areaIRI = _writer->generateIRI(
                    areaFromWay
                        ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                    areaId);
                _writer->writeTriple(areaIRI,
                                     osm2ttl::ttl::constants::IRI__OGC_CONTAINS,
                                     wayIRI);
                _writer->writeTriple(
                    wayIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                    areaIRI);
              }
            }
          }
          progressBar.update(entryCount++);
        }
      }
    }
    progressBar.done();
    std::cerr << " ... done" << std::endl;
  }
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;
