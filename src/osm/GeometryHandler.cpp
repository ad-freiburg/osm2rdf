// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "boost/geometry.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/iostreams/filter/bzip2.hpp"
#include "boost/thread.hpp"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Constants.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/DirectedGraph.h"
#include "osm2ttl/util/Time.h"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/util/progress_bar.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-loop-convert"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::GeometryHandler<W>::GeometryHandler(
    const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {
  std::filesystem::path p{_config.output};
  p += ".stats.bz2";
  _statisticsOut.push(boost::iostreams::bzip2_compressor{});
  _statisticsOutFile.open(p);
  _statisticsOut.push(_statisticsOutFile);
  _statisticsOut << "[" << std::endl;
}

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::GeometryHandler<W>::~GeometryHandler() {
  _statisticsOut << "{}" << std::endl;
  _statisticsOut << "]" << std::endl;
  _statisticsOut.pop();
  _statisticsOutFile.close();
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::writeStatisticLine(
    std::string_view function, std::string_view part, std::string_view check,
    uint64_t outerId, std::string_view outerType, uint64_t innerId,
    std::string_view innerType, std::chrono::nanoseconds durationNS,
    bool result) {
#pragma omp critical(writeStatisticLine)
  _statisticsOut
      << ("{"
          "\"function\":\"" +
          std::string(function) +
          "\","
          "\"part\":\"" +
          std::string(part) +
          "\","
          "\"check\":\"" +
          std::string(check) +
          "\","
          "\"outer_id\":" +
          std::to_string(outerId) +
          ","
          "\"outer_type\":\"" +
          std::string(outerType) +
          "\","
          "\"inner_id\":" +
          std::to_string(innerId) +
          ","
          "\"inner_type\":\"" +
          std::string(innerType) +
          "\","
          "\"duration_ns\":" +
          std::to_string(durationNS.count()) +
          ","
          "\"result\":" +
          (result ? "true" : "false") +
          ""
          "},\n");
}

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
#pragma omp critical(areaDataInsert)
  {
    _areaData[a.id()] = _spatialStorageArea.size();
    _spatialStorageArea.emplace_back(a.envelope(), a.id(), a.geom(), a.objId(),
                                     boost::geometry::area(a.geom()),
                                     a.fromWay());
  }
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
  std::vector<uint64_t> nodeIds;
  nodeIds.reserve(w.nodes().size());
  for (const auto& nodeRef : w.nodes()) {
    nodeIds.push_back(nodeRef.id());
  }
  _spatialStorageWay.emplace_back(w.envelope(), w.id(), w.geom(), nodeIds);
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::calculateRelations() {
  if (_config.noContains) {
    return;
  }
  // Store areas as r-tree
  SpatialIndex spatialIndex;
  // Store dag
  osm2ttl::util::DirectedGraph directedAreaGraph;
  osm2ttl::util::DirectedGraph tmpDirectedAreaGraph;
  // Store for each node all relevant areas
  std::unordered_map<uint64_t, std::vector<uint64_t>> nodeData;
  {
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Packing combined tree with " << _spatialStorageArea.size()
              << " entries ... " << std::endl;
    spatialIndex =
        SpatialIndex(_spatialStorageArea.begin(), _spatialStorageArea.end());
    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Generating non-reduced DAG from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osmium::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;
    size_t checks = 0;
    size_t contains = 0;
    size_t containsOk = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);
    // Shuffle nodes to improve parallel workloads
    std::shuffle(_spatialStorageArea.begin(), _spatialStorageArea.end(),
                 std::mt19937(std::random_device()()));
#pragma omp parallel for shared(spatialIndex, tmpDirectedAreaGraph,          \
    entryCount, progressBar) reduction(+:checks, skippedByDAG, contains,     \
    containsOk) default(none)
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
        checks++;

        if (skip.find(areaId) != skip.end()) {
          skippedByDAG++;
          continue;
        }

        contains++;
        auto start = std::chrono::steady_clock::now();
        bool isCoveredBy = boost::geometry::covered_by(entryGeom, areaGeom);
        auto end = std::chrono::steady_clock::now();
        writeStatisticLine(__func__, "DAG", "isCoveredBy", areaId, "area",
                           entryId, "area",
                           std::chrono::nanoseconds(end - start), isCoveredBy);
        if (isCoveredBy) {
          containsOk++;
          start = std::chrono::steady_clock::now();
          bool isEqual = boost::geometry::equals(entryGeom, areaGeom);
          end = std::chrono::steady_clock::now();
          writeStatisticLine(__func__, "DAG", "isEqual", areaId, "area",
                             entryId, "area",
                             std::chrono::nanoseconds(end - start), isEqual);
          if (!isEqual) {
#pragma omp critical(addEdge)
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

    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done with "
              << checks << " checks, " << skippedByDAG << " skipped by DAG"
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << (checks - skippedByDAG) << " checks performed" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " contains: " << contains
              << " yes: " << containsOk << std::endl;
  }
  if (_config.writeDotFiles) {
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Dumping non-reduced DAG as " << _config.output
              << ".non-reduced.dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".non-reduced.dot";
    tmpDirectedAreaGraph.dump(p);
    std::cerr << osm2ttl::util::currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " Reducing DAG with "
              << tmpDirectedAreaGraph.getNumEdges() << " edges and "
              << tmpDirectedAreaGraph.getNumVertices() << " vertices ... "
              << std::endl;

    // Prepare non-reduced DAG for cleanup
    tmpDirectedAreaGraph.sort();
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " ... adjacency lists sorted ... " << std::endl;

    osmium::ProgressBar progressBar{tmpDirectedAreaGraph.getNumVertices(),
                                    true};
    size_t entryCount = 0;
    progressBar.update(entryCount);
    // Reduce each adjacency list
    const auto& vertices = tmpDirectedAreaGraph.getVertices();
#pragma omp parallel for shared(vertices, tmpDirectedAreaGraph, \
                                directedAreaGraph, progressBar, \
                                entryCount) default(none)
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
#pragma omp critical(addEdge)
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
    std::cerr << osm2ttl::util::currentTimeFormatted() << " Dumping DAG as "
              << _config.output << ".dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".dot";
    directedAreaGraph.dump(p);
    std::cerr << osm2ttl::util::currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Dumping relations from DAG with "
              << directedAreaGraph.getNumEdges() << " edges and "
              << directedAreaGraph.getNumVertices() << " vertices ... "
              << std::endl;

    osmium::ProgressBar progressBar{directedAreaGraph.getNumVertices(), true};
    size_t entryCount = 0;
    progressBar.update(entryCount);
    std::vector<uint64_t> vertices = directedAreaGraph.getVertices();
#pragma omp parallel for shared(                                         \
    vertices, osm2ttl::ttl::constants::NAMESPACE__OSM_WAY,               \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, directedAreaGraph, \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                          \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,                      \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                        \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY, progressBar,       \
    entryCount) default(none)
    for (size_t i = 0; i < vertices.size(); i++) {
      const auto id = vertices[i];
      const auto& entry = _spatialStorageArea[_areaData[id]];
      const auto& entryObjId = std::get<3>(entry);
      const auto& entryFromWay = std::get<5>(entry);
      std::string entryIRI = _writer->generateIRI(
          entryFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                       : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
          entryObjId);
      for (const auto& dst : directedAreaGraph.getEdges(id)) {
        const auto& area = _spatialStorageArea[_areaData[dst]];
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromWay = std::get<5>(area);
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

    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Preparing fast above lookup in DAG ..." << std::endl;
    directedAreaGraph.prepareFastAbove();
    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
  if (!_config.noNodeDump) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Contains relations for " << _spatialStorageNode.size()
              << " nodes in " << spatialIndex.size() << " areas ..."
              << std::endl;

    osmium::ProgressBar progressBar{_spatialStorageNode.size(), true};
    size_t entryCount = 0;
    size_t checks = 0;
    size_t contains = 0;
    size_t containsOk = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);
    // Shuffle nodes to improve parallel workloads
    std::shuffle(_spatialStorageNode.begin(), _spatialStorageNode.end(),
                 std::mt19937(std::random_device()()));
#pragma omp parallel for shared(                                            \
    osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, spatialIndex,             \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY,                            \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                       \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                             \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,                         \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                           \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY, nodeData,             \
    directedAreaGraph, progressBar, entryCount) reduction(+:checks,         \
    skippedByDAG, contains, containsOk) default(none)
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
        checks++;

        if (skip.find(areaId) != skip.end()) {
          skippedByDAG++;
          continue;
        }
        contains++;
        auto start = std::chrono::steady_clock::now();
        bool isCoveredBy = boost::geometry::covered_by(nodeGeom, areaGeom);
        auto end = std::chrono::steady_clock::now();
        writeStatisticLine(__func__, "Node", "isCoveredBy", areaId, "area",
                           nodeId, "node",
                           std::chrono::nanoseconds(end - start), isCoveredBy);
        if (isCoveredBy) {
          containsOk++;
          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
              areaObjId);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, nodeIRI);
          _writer->writeTriple(
              nodeIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, areaIRI);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, nodeIRI);
          _writer->writeTriple(nodeIRI,
                               osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                               areaIRI);
#pragma omp critical(nodeDataChange)
          nodeData[nodeId].push_back(areaId);
          for (const auto& newSkip : directedAreaGraph.findAbove(areaId)) {
            skip.insert(newSkip);
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done with "
              << checks << " checks, " << skippedByDAG << " skipped by DAG"
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << (checks - skippedByDAG) << " checks performed" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " contains: " << contains
              << " yes: " << containsOk << std::endl;
  }

  if (!_config.noWayDump) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Contains relations for " << _spatialStorageWay.size()
              << " ways in " << spatialIndex.size() << " areas ..."
              << std::endl;

    osmium::ProgressBar progressBar{_spatialStorageWay.size(), true};
    size_t entryCount = 0;
    size_t checks = 0;
    size_t intersects = 0;
    size_t intersectsOk = 0;
    size_t intersectsByNodeInfo = 0;
    size_t contains = 0;
    size_t containsOk = 0;
    size_t skippedByDAG = 0;
    size_t containsOkEnvelope = 0;
    progressBar.update(entryCount);
    // Shuffle ways to improve parallel workloads
    std::shuffle(_spatialStorageWay.begin(), _spatialStorageWay.end(),
                 std::mt19937(std::random_device()()));
#pragma omp parallel for shared(                                             \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, nodeData,                   \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                        \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                            \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,                        \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                              \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, directedAreaGraph,       \
    spatialIndex,  progressBar, entryCount) reduction(+:checks,skippedByDAG, \
    intersectsByNodeInfo, intersects, intersectsOk, contains, containsOk, containsOkEnvelope)    \
    default(none)
    for (size_t i = 0; i < _spatialStorageWay.size(); i++) {
      const auto& way = _spatialStorageWay[i];
      const auto& wayEnvelope = std::get<0>(way);
      const auto& wayId = std::get<1>(way);
      const auto& wayGeom = std::get<2>(way);
      const auto& wayNodeIds = std::get<3>(way);
      std::string wayIRI = _writer->generateIRI(
          osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, wayId);

      // Set containing all areas we are inside of
      std::set<uint64_t> skip;

      // Check for known inclusion in areas through containing nodes
      for (const auto& nodeId : wayNodeIds) {
        auto nodeDataIt = nodeData.find(nodeId);
        if (nodeDataIt != nodeData.end()) {
          for (const auto& areaId : nodeDataIt->second) {
            checks++;

            if (skip.find(areaId) != skip.end()) {
              skippedByDAG++;
              continue;
            }

            intersectsByNodeInfo++;
            // Load area data for node entry
            const auto& area = _spatialStorageArea[_areaData[areaId]];
            const auto& areaEnvelope = std::get<0>(area);
            const auto& areaGeom = std::get<2>(area);
            const auto& areaObjId = std::get<3>(area);
            const auto& areaFromWay = std::get<5>(area);
            std::string areaIRI = _writer->generateIRI(
                areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                            : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                areaObjId);
            _writer->writeTriple(
                areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, wayIRI);
            _writer->writeTriple(
                wayIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                areaIRI);
            contains++;
            auto start = std::chrono::steady_clock::now();
            bool isCoveredByEnvelope =
                boost::geometry::covered_by(wayEnvelope, areaEnvelope);
            auto end = std::chrono::steady_clock::now();
            writeStatisticLine(__func__, "Way", "isCoveredByEnvelope", areaId,
                               "area", wayId, "way",
                               std::chrono::nanoseconds(end - start),
                               isCoveredByEnvelope);
            if (isCoveredByEnvelope) {
              containsOkEnvelope++;
              start = std::chrono::steady_clock::now();
              bool isCoveredBy = boost::geometry::covered_by(wayGeom, areaGeom);
              end = std::chrono::steady_clock::now();
              writeStatisticLine(
                  __func__, "Way", "isCoveredBy1", areaId, "area", wayId, "way",
                  std::chrono::nanoseconds(end - start), isCoveredBy);
              if (isCoveredBy) {
                containsOk++;
                _writer->writeTriple(areaIRI,
                                     osm2ttl::ttl::constants::IRI__OGC_CONTAINS,
                                     wayIRI);
                _writer->writeTriple(
                    wayIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                    areaIRI);
              }
            }
            for (const auto& newSkip : directedAreaGraph.findAbove(areaId)) {
              skip.insert(newSkip);
            }
          }
        }
      }

      std::vector<SpatialAreaValue> queryResult;
      spatialIndex.query(boost::geometry::index::intersects(wayEnvelope),
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
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromWay = std::get<5>(area);
        if (areaFromWay && areaObjId == wayId) {
          continue;
        }
        checks++;

        if (skip.find(areaId) != skip.end()) {
          skippedByDAG++;
          continue;
        }

        intersects++;
        auto start = std::chrono::steady_clock::now();
        bool doesIntersect = boost::geometry::intersects(wayGeom, areaGeom);
        auto end = std::chrono::steady_clock::now();
        writeStatisticLine(__func__, "Way", "doesIntersect", areaId, "area",
                           wayId, "way", std::chrono::nanoseconds(end - start),
                           doesIntersect);
        if (doesIntersect) {
          intersectsOk++;
          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
              areaObjId);
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, wayIRI);
          _writer->writeTriple(wayIRI,
                               osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                               areaIRI);
          contains++;
          start = std::chrono::steady_clock::now();
          bool isCoveredByEnvelope =
              boost::geometry::covered_by(wayEnvelope, areaEnvelope);
          end = std::chrono::steady_clock::now();
          writeStatisticLine(__func__, "Way", "isCoveredByEnvelope", areaId,
                             "area", wayId, "way",
                             std::chrono::nanoseconds(end - start),
                             isCoveredByEnvelope);
          if (isCoveredByEnvelope) {
            containsOkEnvelope++;
            start = std::chrono::steady_clock::now();
            bool isCoveredBy = boost::geometry::covered_by(wayGeom, areaGeom);
            end = std::chrono::steady_clock::now();
            writeStatisticLine(
                __func__, "Way", "isCoveredBy2", areaId, "area", wayId, "way",
                std::chrono::nanoseconds(end - start), isCoveredBy);
            if (isCoveredBy) {
              containsOk++;
              _writer->writeTriple(
                  areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, wayIRI);
              _writer->writeTriple(
                  wayIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                  areaIRI);
            }
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

    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done with "
              << checks << " checks, " << skippedByDAG << " skipped by DAG"
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << (checks - skippedByDAG) << " checks performed" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer
              << " intersect info by nodes: " << intersectsByNodeInfo
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer
              << " intersect: " << intersects << " yes: " << intersectsOk
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " contains: " << contains
              << " contains envelope: " << containsOkEnvelope
              << " yes: " << containsOk << std::endl;
  }
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;

#pragma clang diagnostic pop
