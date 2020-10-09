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
#include "osm2ttl/util/DirectedGraph.h"
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
                                   a.fromWay(),
                                   boost::geometry::area(a.geom()));
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
  osm2ttl::util::DirectedGraph directedAreaGraph;
  {
    std::cerr << " Packing combined tree with " << _spatialStorageArea.size()
              << " entries ... " << std::flush;
    spatialIndex =
        SpatialIndex(_spatialStorageArea.begin(), _spatialStorageArea.end());
    std::cerr << "done" << std::endl;
  }
  {
    std::cerr << " Sorting " << _spatialStorageArea.size()
              << " areas by explicit size ... " << std::flush;
    std::vector<SpatialAreaValue> sav(_spatialStorageArea.begin(),
                                      _spatialStorageArea.end());
    // big -> small
    std::sort(sav.begin(), sav.end(), [](const auto& a, const auto& b) {
      return std::get<4>(a) < std::get<4>(b);
    });
    std::cerr << " done" << std::endl;
    std::cerr << " Generating DAG from " << sav.size() << " sorted areas ... "
              << std::endl;

    osmium::ProgressBar progressBar{sav.size(), true};
    size_t entryCount = 0;
    size_t skippedByDAG = 0;
    size_t skippedByEnvelope = 0;
    progressBar.update(entryCount);

    for (auto& entry : sav) {
      const auto& entryEnvelope = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryGeom = std::get<2>(entry);
      std::string entryIRI = _writer->generateIRI(
          std::get<3>(entry) ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                             : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
          std::get<1>(entry));
      // Set containing all areas we are inside of
      std::set<uint64_t> skip;
      std::vector<SpatialAreaValue> queryResult;
      spatialIndex.query(boost::geometry::index::covers(entryEnvelope),
                         std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.rbegin(), queryResult.rend(),
                [](const auto& a, const auto& b) {
                  return std::get<4>(a) < std::get<4>(b);
                });

      for (const auto& area : queryResult) {
        const auto& areaEnvelope = std::get<0>(area);
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaFromWay = std::get<3>(area);
        if (areaId == entryId) {
          continue;
        }

        if (!boost::geometry::covered_by(entryEnvelope,
                                         areaEnvelope)) {
          skippedByEnvelope++;
          continue;
        }
        if (skip.find(areaId) != skip.end()) {
          skippedByDAG++;
          continue;
        }

        if (boost::geometry::covered_by(entryGeom,
                                        areaGeom)) {
          if (!boost::geometry::equals(entryGeom, areaGeom)) {
            directedAreaGraph.addEdge(entryId, areaId);
            std::string areaIRI = _writer->generateIRI(
                areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                            : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                areaId);
            _writer->writeTriple(
                areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, entryIRI);
            _writer->writeTriple(entryIRI,
                                 osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                                 areaIRI);
            for (const auto& newSkip :
                 directedAreaGraph.findAbove(entryId)) {
              skip.insert(newSkip);
            }
          }
        }
      }
      progressBar.update(entryCount++);
    }

    progressBar.done();
    std::cerr << " ... done with " << skippedByDAG << " skipped checks DAG and "
              << skippedByEnvelope << " skipped by Envelope" << std::endl;
  }

  if (!_config.noNodeDump) {
    std::cerr << " Contains relations for " << _spatialStorageNode.size()
              << " nodes in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageNode.size(), true};
    size_t entryCount = 0;
    size_t skippedByDAG = 0;
    size_t skippedByEnvelope = 0;
    progressBar.update(entryCount);

#pragma omp parallel for
    for (size_t i = 0; i < _spatialStorageNode.size(); i++) {
      const auto& node = _spatialStorageNode[i];
      const auto& nodeEnvelope = std::get<0>(node);
      const auto& nodeId = std::get<1>(node);
      const auto& nodeGeom = std::get<2>(node);
      std::string nodeIRI = _writer->generateIRI(
          osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, nodeId);

      // Set containing all areas we are inside of
      std::set<uint64_t> skip;
      std::vector<SpatialAreaValue> queryResult;
      spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                         std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.rbegin(), queryResult.rend(),
                [](const auto& a, const auto& b) {
                  return std::get<4>(a) < std::get<4>(b);
                });
      for (const auto& area : queryResult) {
        const auto& areaEnvelope = std::get<0>(area);
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaFromWay = std::get<3>(area);

        if (!boost::geometry::covered_by(nodeEnvelope, areaEnvelope)) {
          skippedByEnvelope++;
          continue;
        }
        if (skip.find(areaId) != skip.end()) {
          skippedByDAG++;
          continue;
        }

        if (boost::geometry::covered_by(nodeGeom, areaGeom)) {
          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
              areaId);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, nodeIRI);
          _writer->writeTriple(
              nodeIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, areaIRI);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, nodeIRI);
          _writer->writeTriple(
              nodeIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY, areaIRI);
          for (const auto& newSkip :
               directedAreaGraph.findAbove(std::get<1>(area))) {
            skip.insert(newSkip);
          }
        }
      }
      progressBar.update(entryCount++);
    }
    progressBar.done();
    std::cerr << " ... done with " << skippedByDAG << " skipped checks DAG and "
              << skippedByEnvelope << " skipped by Envelope" << std::endl;
  }

  if (!_config.noWayDump) {
    std::cerr << " Contains relations for " << _spatialStorageWay.size()
              << " ways in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageWay.size(), true};
    size_t entryCount = 0;
    size_t skippedByDAG = 0;
    size_t skippedByEnvelope = 0;
    progressBar.update(entryCount);

#pragma omp parallel for
    for (size_t i = 0; i < _spatialStorageWay.size(); i++) {
      const auto& way = _spatialStorageWay[i];
      const auto& wayEnvelope = std::get<0>(way);
      const auto& wayId = std::get<1>(way);
      const auto& wayGeom = std::get<2>(way);
      std::string wayIRI = _writer->generateIRI(
          osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, wayId);

      // Set containing all areas we are inside of
      std::set<uint64_t> skip;
      std::vector<SpatialAreaValue> queryResult;
      spatialIndex.query(boost::geometry::index::covers(wayEnvelope),
                         std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.rbegin(), queryResult.rend(),
                [](const auto& a, const auto& b) {
                  return std::get<4>(a) < std::get<4>(b);
                });
      for (const auto& area : queryResult) {
        const auto& areaEnvelope = std::get<0>(area);
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaFromWay = std::get<3>(area);
        if (!areaFromWay || areaId != wayId) {
          continue;
        }

        if (!boost::geometry::covered_by(wayEnvelope, areaEnvelope)) {
          skippedByEnvelope++;
          continue;
        }
        if (skip.find(areaId) != skip.end()) {
          skippedByDAG++;
          continue;
        }

        if (boost::geometry::covered_by(wayGeom, areaGeom)) {
          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
              areaId);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, wayIRI);
          _writer->writeTriple(
              wayIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, areaIRI);
          for (const auto& newSkip :
               directedAreaGraph.findAbove(std::get<1>(area))) {
            skip.insert(newSkip);
          }
        }
      }
      progressBar.update(entryCount++);
    }
    progressBar.done();
    std::cerr << " ... done with " << skippedByDAG << " skipped checks DAG and "
              << skippedByEnvelope << " skipped by Envelope" << std::endl;
  }
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;
