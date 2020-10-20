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
#include "osm2ttl/util/Time.h"
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
  _spatialStorageArea.emplace_back(a.envelope(), a.id(), a.geom(), a.objId(),
                                   boost::geometry::area(a.geom()),
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
  osm2ttl::util::DirectedGraph directedAreaGraph;
  osm2ttl::util::DirectedGraph tmpDirectedAreaGraph;
  {
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Packing combined tree with " << _spatialStorageArea.size()
              << " entries ... " << std::endl;
    spatialIndex =
        SpatialIndex(_spatialStorageArea.begin(), _spatialStorageArea.end());
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " ... done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Generating non-reduced DAG from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osmium::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;
    size_t numberChecks = 0;
    size_t okIntersects = 0;
    size_t okContains = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);

    // Shuffle nodes to improve parallel workloads
    std::random_shuffle(_spatialStorageArea.begin(), _spatialStorageArea.end());

#pragma omp parallel for
    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryEnvelope = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryGeom = std::get<2>(entry);
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
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        if (areaId == entryId) {
          continue;
        }
#pragma omp atomic
        numberChecks++;

        if (skip.find(areaId) != skip.end()) {
#pragma omp atomic
          skippedByDAG++;
          continue;
        }

        if (boost::geometry::covered_by(entryGeom, areaGeom)) {
#pragma omp atomic
          okContains++;
          if (!boost::geometry::equals(entryGeom, areaGeom)) {
            tmpDirectedAreaGraph.addEdge(entryId, areaId);
            for (const auto& newSkip :
                 tmpDirectedAreaGraph.findAbove(entryId)) {
              skip.insert(newSkip);
            }
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }

    progressBar.done();
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " ... done with " << numberChecks << " checks, "
              << skippedByDAG << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer
              << " " << (numberChecks - skippedByDAG)
              << " checks performed, intersects: " << okIntersects
              << ", contains: " << okContains << std::endl;
  }
  if (_config.writeDotFiles) {
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Dumping non-reduced DAG as " << _config.output
              << ".non-reduced.dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".non-reduced.dot";
    tmpDirectedAreaGraph.dump(p);
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Reducing DAG with " << tmpDirectedAreaGraph.getNumEdges()
              << " edges and " << tmpDirectedAreaGraph.getNumVertices()
              << " vertices ... " << std::endl;

    osmium::ProgressBar progressBar{tmpDirectedAreaGraph.getNumVertices(),
                                    true};
    size_t entryCount = 0;
    progressBar.update(entryCount);

    // Prepare non-reduced DAG for cleanup
    tmpDirectedAreaGraph.sort();
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " ... adjacency lists sorted ... " << std::endl;

    // Reduce each adjacency list
    const auto& vertices = tmpDirectedAreaGraph.getVertices();
#pragma omp parallel for
    for (size_t i = 0; i < vertices.size(); i++) {
      uint64_t src = vertices[i];
      std::vector<uint64_t> possibleEdges = tmpDirectedAreaGraph.getEdges(src);
      std::vector<uint64_t> edges;
      for (const auto& dst : tmpDirectedAreaGraph.getEdges(src)) {
          const auto& dstEdges = tmpDirectedAreaGraph.getEdges(dst);
          std::set_difference(possibleEdges.begin(), possibleEdges.end(),
                              dstEdges.begin(), dstEdges.end(),
                              std::back_inserter(edges));
          possibleEdges = edges;
          edges.clear();
        }
      for (const auto& dst : possibleEdges) {
        directedAreaGraph.addEdge(src, dst);
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }

    progressBar.done();
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " ... done, resulting in DAG with "
              << directedAreaGraph.getNumEdges() << " edges and "
              << directedAreaGraph.getNumVertices() << " vertices" << std::endl;
  }
  if (_config.writeDotFiles) {
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Dumping DAG as " << _config.output << ".dot ..."
              << std::endl;
    std::filesystem::path p{_config.output};
    p += ".dot";
    directedAreaGraph.dump(p);
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Preparing area data for dump ..." << std::flush;
    std::unordered_map<uint64_t, std::pair<uint64_t, bool>> areaData;
    for (const auto& area : _spatialStorageArea) {
      areaData[std::get<1>(area)] =
          std::make_pair(std::get<3>(area), std::get<5>(area));
    }
    std::cerr << " done" << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Dumping relations from DAG with "
              << directedAreaGraph.getNumEdges() << " edges and "
              << directedAreaGraph.getNumVertices() << " vertices ... "
              << std::endl;

    osmium::ProgressBar progressBar{directedAreaGraph.getNumVertices(), true};
    size_t entryCount = 0;
    progressBar.update(entryCount);

    std::vector<uint64_t> vertices = directedAreaGraph.getVertices();
#pragma omp parallel for
    for (size_t i = 0; i < vertices.size(); i++) {
      const auto id = vertices[i];
      const auto& entry = areaData.at(id);
      const auto& entryObjId = entry.first;
      const auto& entryFromWay = entry.second;
      std::string entryIRI = _writer->generateIRI(
          entryFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                       : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
          entryObjId);
      for (const auto& dst : tmpDirectedAreaGraph.getEdges(id)) {
        const auto& area = areaData.at(dst);
        const auto& areaObjId = area.first;
        const auto& areaFromWay = area.second;
        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);

        _writer->writeTriple(
            areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, entryIRI);
        _writer->writeTriple(
            entryIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, areaIRI);
        _writer->writeTriple(
            areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, entryIRI);
        _writer->writeTriple(entryIRI,
                             osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                             areaIRI);
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }

    progressBar.done();
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " ... done" << std::endl;
  }

  if (!_config.noNodeDump) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Contains relations for " << _spatialStorageNode.size()
              << " nodes in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageNode.size(), true};
    size_t entryCount = 0;
    size_t numberChecks = 0;
    size_t okContains = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);

    // Shuffle nodes to improve parallel workloads
    std::random_shuffle(_spatialStorageNode.begin(), _spatialStorageNode.end());
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
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromWay = std::get<5>(area);
#pragma omp atomic
        numberChecks++;

        if (skip.find(areaId) != skip.end()) {
#pragma omp atomic
          skippedByDAG++;
          continue;
        }
        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);
        if (boost::geometry::covered_by(nodeGeom, areaGeom)) {
#pragma omp atomic
          okContains++;
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, nodeIRI);
          _writer->writeTriple(
              nodeIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, areaIRI);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, nodeIRI);
          _writer->writeTriple(nodeIRI,
                               osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                               areaIRI);
          for (const auto& newSkip : directedAreaGraph.findAbove(areaId)) {
            skip.insert(newSkip);
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " ... done with " << numberChecks << " checks, "
              << skippedByDAG << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer
              << " " << (numberChecks - skippedByDAG)
              << " checks performed, contains: " << okContains << std::endl;
  }

  if (!_config.noWayDump) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " Contains relations for " << _spatialStorageWay.size()
              << " ways in " << spatialIndex.size() << " areas ..."
              << std::endl;
    osmium::ProgressBar progressBar{_spatialStorageWay.size(), true};
    size_t entryCount = 0;
    size_t numberChecks = 0;
    size_t okIntersects = 0;
    size_t okContains = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);

    // Shuffle ways to improve parallel workloads
    std::random_shuffle(_spatialStorageWay.begin(), _spatialStorageWay.end());
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
      spatialIndex.query(boost::geometry::index::intersects(wayEnvelope),
                         std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.rbegin(), queryResult.rend(),
                [](const auto& a, const auto& b) {
                  return std::get<4>(a) < std::get<4>(b);
                });
      for (const auto& area : queryResult) {
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromWay = std::get<5>(area);
        if (areaFromWay && areaObjId == wayId) {
          continue;
        }
#pragma omp atomic
        numberChecks++;

        if (skip.find(areaId) != skip.end()) {
#pragma omp atomic
          skippedByDAG++;
          continue;
        }

        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);
        if (boost::geometry::intersects(wayGeom, areaGeom)) {
#pragma omp atomic
          okIntersects++;
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, wayIRI);
          _writer->writeTriple(wayIRI,
                               osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                               areaIRI);
          if (boost::geometry::covered_by(wayGeom, areaGeom)) {
#pragma omp atomic
            okContains++;
            _writer->writeTriple(
                areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, wayIRI);
            _writer->writeTriple(wayIRI,
                                 osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                                 areaIRI);
          }
          for (const auto& newSkip : directedAreaGraph.findAbove(areaId)) {
            skip.insert(newSkip);
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();
    std::cerr << osm2ttl::util::currentTimeFormatted() 
              << " ... done with " << numberChecks << " checks, "
              << skippedByDAG << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer
              << " " << (numberChecks - skippedByDAG)
              << " checks performed, intersects: " << okIntersects
              << ", contains: " << okContains << std::endl;
  }
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;
