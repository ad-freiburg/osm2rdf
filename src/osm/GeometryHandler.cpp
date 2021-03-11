// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "boost/archive/binary_iarchive.hpp"
#include "boost/geometry.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/thread.hpp"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Constants.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/DirectedAcyclicGraph.h"
#include "osm2ttl/util/DirectedGraph.h"
#include "osm2ttl/util/ProgressBar.h"
#include "osm2ttl/util/Time.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-loop-convert"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::GeometryHandler<W>::GeometryHandler(
    const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer),
      _statistics(config, config.statisticsPath.string()),
      _ofsUnnamedAreas(config.getTempPath("spatial", "areas_unnamed"),
                       std::ios::binary),
      _oaUnnamedAreas(_ofsUnnamedAreas),
      _ofsNodes(config.getTempPath("spatial", "nodes"), std::ios::binary),
      _oaNodes(_ofsNodes),
      _ofsWays(config.getTempPath("spatial", "ways"), std::ios::binary),
      _oaWays(_ofsWays) {
  _ofsUnnamedAreas << std::fixed << std::setprecision(_config.wktPrecision);
  _ofsWays << std::fixed << std::setprecision(_config.wktPrecision);
  _ofsNodes << std::fixed << std::setprecision(_config.wktPrecision);
}

// ___________________________________________________________________________
template <typename W>
osm2ttl::osm::GeometryHandler<W>::~GeometryHandler() = default;

// ____________________________________________________________________________
template <typename W>
std::string osm2ttl::osm::GeometryHandler<W>::statisticLine(
    std::string_view function, std::string_view part, std::string_view check,
    uint64_t outerId, std::string_view outerType, uint64_t innerId,
    std::string_view innerType, std::chrono::nanoseconds durationNS,
    bool result) {
  return "{"
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
         "},\n";
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::area(const osm2ttl::osm::Area& area) {
#pragma omp critical(areaDataInsert)
  {
    if (area.hasName()) {
      _spatialStorageArea.push_back(
          SpatialAreaValue(area.envelope(), area.id(), area.geom(),
                           area.objId(), area.geomArea(), area.fromWay()));
    } else if (!area.fromWay()) {
      // Areas from ways are handled in GeometryHandler<W>::way
      _oaUnnamedAreas << SpatialAreaValue(area.envelope(), area.id(),
                                          area.geom(), area.objId(),
                                          area.geomArea(), area.fromWay());
      _numUnnamedAreas++;
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::node(const osm2ttl::osm::Node& node) {
#pragma omp critical(nodeDataInsert)
  {
    _oaNodes << SpatialNodeValue(node.envelope(), node.id(), node.geom());
    _numNodes++;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::way(const osm2ttl::osm::Way& way) {
  std::vector<uint64_t> nodeIds;
  nodeIds.reserve(way.nodes().size());
  for (const auto& nodeRef : way.nodes()) {
    nodeIds.push_back(nodeRef.id());
  }
#pragma omp critical(wayDataInsert)
  {
    _oaWays << SpatialWayValue(way.envelope(), way.id(), way.geom(), nodeIds);
    _numWays++;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::closeExternalStorage() {
  if (_ofsNodes.is_open()) {
    _ofsNodes.close();
  }
  if (_ofsUnnamedAreas.is_open()) {
    _ofsUnnamedAreas.close();
  }
  if (_ofsWays.is_open()) {
    _ofsWays.close();
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::calculateRelations() {
  // Ensure functions can open external storage for reading.
  closeExternalStorage();
  if (_config.writeGeometricRelationStatistics) {
    _statistics.open();
  }
  prepareRTree();
  prepareDAG();
  dumpNamedAreaRelations();
  dumpUnnamedAreaRelations();
  const auto& nodeData = dumpNodeRelations();
  dumpWayRelations(nodeData);

  if (_config.writeGeometricRelationStatistics) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Closing statistics files..." << std::endl;
    _statistics.close("[\n", "{}\n]");
    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::prepareRTree() {
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << " Packing area r-tree with " << _spatialStorageArea.size()
            << " entries ... " << std::endl;
  _spatialIndex =
      SpatialIndex(_spatialStorageArea.begin(), _spatialStorageArea.end());
  std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
            << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::prepareDAG() {
  // Store dag
  osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t> tmpDirectedAreaGraph;
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " Sorting "
              << _spatialStorageArea.size() << " areas ... " << std::endl;
    std::sort(_spatialStorageArea.begin(), _spatialStorageArea.end(),
              [](const auto& a, const auto& b) {
                return std::get<4>(a) < std::get<4>(b);
              });
    // Prepare id based lookup table for later usage...
    _spatialStorageAreaIndex.reserve(_spatialStorageArea.size());
    for (size_t i = 0; i < _spatialStorageArea.size(); ++i) {
      const auto& area = _spatialStorageArea[i];
      _spatialStorageAreaIndex[std::get<1>(area)] = i;
    }
    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done "
              << std::endl;

    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Generating non-reduced DAG from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osm2ttl::util::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;
    size_t checks = 0;
    size_t contains = 0;
    size_t containsOk = 0;
    size_t skippedByDAG = 0;
    size_t skippedBySize = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared(tmpDirectedAreaGraph,           \
    entryCount, progressBar) reduction(+:checks, skippedBySize, skippedByDAG, \
    contains, containsOk) default(none) schedule(dynamic)
    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryEnvelope = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryGeom = std::get<2>(entry);
      // Set containing all areas we are inside of
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skip;
      std::vector<SpatialAreaValue> queryResult;
      _spatialIndex.query(boost::geometry::index::covers(entryEnvelope),
                          std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
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
#ifdef ENABLE_GEOMETRY_STATISTIC
        auto start = std::chrono::steady_clock::now();
#endif
        bool isCoveredBy = boost::geometry::covered_by(entryGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
        auto end = std::chrono::steady_clock::now();
        if (_config.writeGeometricRelationStatistics) {
          _statistics.write(statisticLine(
              __func__, "DAG", "isCoveredBy", areaId, "area", entryId, "area",
              std::chrono::nanoseconds(end - start), isCoveredBy));
        }
#endif
        if (!isCoveredBy) {
          continue;
        }
        containsOk++;
#ifdef ENABLE_GEOMETRY_STATISTIC
        start = std::chrono::steady_clock::now();
#endif
        bool isEqual = boost::geometry::equals(entryGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
        end = std::chrono::steady_clock::now();
        if (_config.writeGeometricRelationStatistics) {
          _statistics.write(statisticLine(
              __func__, "DAG", "isEqual", areaId, "area", entryId, "area",
              std::chrono::nanoseconds(end - start), isEqual));
        }
#endif
        if (isEqual) {
          continue;
        }
#pragma omp critical(addEdge)
        tmpDirectedAreaGraph.addEdge(entryId, areaId);
        for (const auto& newSkip :
             tmpDirectedAreaGraph.findSuccessors(entryId)) {
          skip.insert(newSkip);
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done with "
              << checks << " checks, " << skippedBySize << " skipped by Size,"
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << skippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << (checks - (skippedByDAG + skippedBySize))
              << " checks performed" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " contains: " << contains
              << " yes: " << containsOk << std::endl;
  }
  if (_config.writeDAGDotFiles) {
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
    tmpDirectedAreaGraph.prepareFindSuccessorsFast();
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " ... fast lookup prepared ... " << std::endl;

    _directedAreaGraph = osm2ttl::util::reduceDAG(tmpDirectedAreaGraph, true);

    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " ... done, resulting in DAG with "
              << _directedAreaGraph.getNumEdges() << " edges and "
              << _directedAreaGraph.getNumVertices() << " vertices"
              << std::endl;
  }
  if (_config.writeDAGDotFiles) {
    std::cerr << osm2ttl::util::currentTimeFormatted() << " Dumping DAG as "
              << _config.output << ".dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".dot";
    _directedAreaGraph.dump(p);
    std::cerr << osm2ttl::util::currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << " Preparing fast above lookup in DAG ..." << std::endl;
    _directedAreaGraph.prepareFindSuccessorsFast();
    std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::dumpNamedAreaRelations() {
  std::cerr << std::endl;
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << " Dumping relations from DAG with "
            << _directedAreaGraph.getNumEdges() << " edges and "
            << _directedAreaGraph.getNumVertices() << " vertices ... "
            << std::endl;

  osm2ttl::util::ProgressBar progressBar{_directedAreaGraph.getNumVertices(),
                                         true};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  std::vector<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
      vertices = _directedAreaGraph.getVertices();
#pragma omp parallel for shared(                                        \
    vertices, osm2ttl::ttl::constants::NAMESPACE__OSM_WAY,              \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                   \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                         \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,                     \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS_AREA,                    \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY_AREA,                \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                       \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,                   \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS_AREA,                  \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY_AREA, progressBar, \
    entryCount) default(none) schedule(static)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto id = vertices[i];
    const auto& entry = _spatialStorageArea[_spatialStorageAreaIndex[id]];
    const auto& entryObjId = std::get<3>(entry);
    const auto& entryFromWay = std::get<5>(entry);
    std::string entryIRI = _writer->generateIRI(
        entryFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                     : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
        entryObjId);
    for (const auto& dst : _directedAreaGraph.getEdges(id)) {
      assert(_spatialStorageAreaIndex[dst] < _spatialStorageArea.size());
      const auto& area = _spatialStorageArea[_spatialStorageAreaIndex[dst]];
      const auto& areaObjId = std::get<3>(area);
      const auto& areaFromWay = std::get<5>(area);
      std::string areaIRI = _writer->generateIRI(
          areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                      : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
          areaObjId);

      _writer->writeTriple(
          areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS_AREA, entryIRI);
      _writer->writeTriple(
          areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS_AREA, entryIRI);
      if (_config.addInverseRelationDirection) {
        _writer->writeTriple(
            entryIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY_AREA,
            areaIRI);
        _writer->writeTriple(
            entryIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY_AREA,
            areaIRI);
      }
    }
#pragma omp critical(progress)
    progressBar.update(entryCount++);
  }

  progressBar.done();

  std::cerr << osm2ttl::util::currentTimeFormatted() << " ... done"
            << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::dumpUnnamedAreaRelations() {
  if (_config.noAreaGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Skipping contains relation for unnamed areas ... disabled"
              << std::endl;
  } else if (_numUnnamedAreas == 0) {
    std::cerr << std::endl;
    std::cerr
        << osm2ttl::util::currentTimeFormatted() << " "
        << "Skipping contains relation for unnamed areas ... no unnamed area"
        << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Contains relations for " << _numUnnamedAreas
              << " unnamed areas in " << _spatialIndex.size() << " areas ..."
              << std::endl;
    std::ifstream ifs(_config.getTempPath("spatial", "areas_unnamed"),
                      std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);

    osm2ttl::util::ProgressBar progressBar{_numUnnamedAreas, true};
    size_t entryCount = 0;
    size_t areas = 0;
    size_t intersectsChecks = 0;
    size_t intersectsOk = 0;
    size_t containsChecks = 0;
    size_t containsOk = 0;
    size_t intersectsSkippedByDAG = 0;
    size_t containsSkippedByDAG = 0;
    size_t containsOkEnvelope = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared(                                             \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY,                             \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                        \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                            \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,                        \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                              \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, \
    progressBar, entryCount, ia) reduction(+:areas,intersectsSkippedByDAG, \
    containsSkippedByDAG, intersectsChecks, intersectsOk, containsChecks, containsOk, \
    containsOkEnvelope)      \
    default(none) schedule(dynamic)
    for (size_t i = 0; i < _numUnnamedAreas; i++) {
      SpatialAreaValue entry;
#pragma omp critical(loadEntry)
      ia >> entry;
      const auto& entryEnvelope = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryGeom = std::get<2>(entry);
      const auto& entryObjId = std::get<3>(entry);
      const auto& entryFromWay = std::get<5>(entry);
      std::string entryIRI = _writer->generateIRI(
          entryFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                       : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
          entryObjId);

      // Set containing all areas we are inside of
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skipIntersects;
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skipContains;

      std::vector<SpatialAreaValue> queryResult;
      _spatialIndex.query(boost::geometry::index::intersects(entryEnvelope),
                          std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
                [](const auto& a, const auto& b) {
                  return std::get<4>(a) < std::get<4>(b);
                });
      for (const auto& area : queryResult) {
        const auto& areaEnvelope = std::get<0>(area);
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromWay = std::get<5>(area);
        if (areaId == entryId) {
          continue;
        }
        areas++;

        bool doesIntersect = false;
        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);

        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          intersectsSkippedByDAG++;
          doesIntersect = true;
        } else {
          intersectsChecks++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto start = std::chrono::steady_clock::now();
#endif
          doesIntersect = boost::geometry::intersects(entryGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Way", "doesIntersect", areaId, "area", entryId,
                "area", std::chrono::nanoseconds(end - start), doesIntersect));
          }
#endif
          if (!doesIntersect) {
            continue;
          }
          intersectsOk++;

          for (const auto& newSkip :
               _directedAreaGraph.findSuccessorsFast(areaId)) {
            skipIntersects.insert(newSkip);
          }
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, entryIRI);
          if (_config.addInverseRelationDirection) {
            _writer->writeTriple(
                entryIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                areaIRI);
          }
        }

        if (!doesIntersect) {
          continue;
        }
        if (skipContains.find(areaId) != skipContains.end()) {
          containsSkippedByDAG++;
        } else {
          containsChecks++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          start = std::chrono::steady_clock::now();
#endif
          bool isCoveredByEnvelope =
              boost::geometry::covered_by(entryEnvelope, areaEnvelope);
#ifdef ENABLE_GEOMETRY_STATISTIC
          end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Way", "isCoveredByEnvelope", areaId, "area", entryId,
                "area", std::chrono::nanoseconds(end - start),
                isCoveredByEnvelope));
          }
#endif
          if (!isCoveredByEnvelope) {
            continue;
          }
          containsOkEnvelope++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          start = std::chrono::steady_clock::now();
#endif
          bool isCoveredBy = boost::geometry::covered_by(entryGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
          end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Way", "isCoveredBy2", areaId, "area", entryId,
                "area", std::chrono::nanoseconds(end - start), isCoveredBy));
          }
#endif
          if (!isCoveredBy) {
            continue;
          }
          containsOk++;

          for (const auto& newSkip :
               _directedAreaGraph.findSuccessorsFast(areaId)) {
            skipContains.insert(newSkip);
          }
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, entryIRI);
          if (_config.addInverseRelationDirection) {
            _writer->writeTriple(entryIRI,
                                 osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                                 areaIRI);
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "... done with looking at " << areas << " areas" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << intersectsChecks
              << " intersection checks performed, " << intersectsSkippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << "intersect: " << intersectsChecks << " yes: " << intersectsOk
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << containsChecks
              << " contains checks performed, " << containsSkippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << "contains: " << containsChecks
              << " contains envelope: " << containsOkEnvelope
              << " yes: " << containsOk << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::NodesContainedInAreasData
osm2ttl::osm::GeometryHandler<W>::dumpNodeRelations() {
  // Store for each node all relevant areas
  NodesContainedInAreasData nodeData;
  if (_config.noNodeGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Skipping contains relation for nodes ... disabled"
              << std::endl;
  } else if (_numNodes == 0) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Skipping contains relation for nodes ... no nodes"
              << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Contains relations for " << _numNodes << " nodes in "
              << _spatialIndex.size() << " areas ..." << std::endl;

    std::ifstream ifs(_config.getTempPath("spatial", "nodes"),
                      std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);

    osm2ttl::util::ProgressBar progressBar{_numNodes, true};
    size_t entryCount = 0;
    size_t checks = 0;
    size_t contains = 0;
    size_t containsOk = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared(                                            \
    osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY,                            \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                       \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                             \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,                         \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                           \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY, nodeData,             \
    progressBar, entryCount, ia) reduction(+:checks,         \
    skippedByDAG, contains, containsOk) default(none) schedule(dynamic)
    for (size_t i = 0; i < _numNodes; i++) {
      SpatialNodeValue node;
#pragma omp critical(loadEntry)
      ia >> node;
      const auto& nodeEnvelope = std::get<0>(node);
      const auto& nodeId = std::get<1>(node);
      const auto& nodeGeom = std::get<2>(node);
      std::string nodeIRI = _writer->generateIRI(
          osm2ttl::ttl::constants::NAMESPACE__OSM_NODE, nodeId);

      // Set containing all areas we are inside of
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skip;
      std::vector<SpatialAreaValue> queryResult;
      _spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                          std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
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
#ifdef ENABLE_GEOMETRY_STATISTIC
        auto start = std::chrono::steady_clock::now();
#endif
        bool isCoveredBy = boost::geometry::covered_by(nodeGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
        auto end = std::chrono::steady_clock::now();
        if (_config.writeGeometricRelationStatistics) {
          _statistics.write(statisticLine(
              __func__, "Node", "isCoveredBy", areaId, "area", nodeId, "node",
              std::chrono::nanoseconds(end - start), isCoveredBy));
        }
#endif
        if (!isCoveredBy) {
          continue;
        }
        containsOk++;
        skip.insert(areaId);
        for (const auto& newSkip :
             _directedAreaGraph.findSuccessorsFast(areaId)) {
          skip.insert(newSkip);
        }
        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);
        _writer->writeTriple(
            areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, nodeIRI);
        _writer->writeTriple(
            areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, nodeIRI);
        if (_config.addInverseRelationDirection) {
          _writer->writeTriple(nodeIRI,
                               osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                               areaIRI);
          _writer->writeTriple(
              nodeIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, areaIRI);
        }
      }
#pragma omp critical(nodeDataChange)
      std::copy(skip.begin(), skip.end(), std::back_inserter(nodeData[nodeId]));
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "... done with looking at " << checks << " areas, "
              << skippedByDAG << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << contains
              << " checks performed" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << "contains: " << contains << " yes: " << containsOk
              << std::endl;
  }
  return nodeData;
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::dumpWayRelations(
    const osm2ttl::osm::NodesContainedInAreasData& nodeData) {
  if (_config.noWayGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Skipping contains relation for ways ... disabled"
              << std::endl;
  } else if (_numWays == 0) {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Skipping contains relation for ways ... no ways" << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "Contains relations for " << _numWays << " ways in "
              << _spatialIndex.size() << " areas ..." << std::endl;

    std::ifstream ifs(_config.getTempPath("spatial", "ways"), std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);

    osm2ttl::util::ProgressBar progressBar{_numWays, true};
    size_t entryCount = 0;
    size_t areas = 0;
    size_t intersectsChecks = 0;
    size_t intersectsOk = 0;
    size_t intersectsSkippedByDAG = 0;
    size_t intersectsByNodeInfo = 0;
    size_t containsChecks = 0;
    size_t containsOk = 0;
    size_t containsOkEnvelope = 0;
    size_t containsSkippedByDAG = 0;
    size_t skippedInDAG = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared(                                             \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, nodeData,                   \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                        \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTS,                            \
    osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,                        \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINS,                              \
    osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY, \
    progressBar, entryCount, ia) reduction(+:areas,intersectsSkippedByDAG, containsSkippedByDAG, skippedInDAG, \
    intersectsByNodeInfo, intersectsChecks, intersectsOk, containsChecks, containsOk, containsOkEnvelope)    \
    default(none) schedule(dynamic)
    for (size_t i = 0; i < _numWays; i++) {
      SpatialWayValue way;
#pragma omp critical(loadEntry)
      ia >> way;
      const auto& wayEnvelope = std::get<0>(way);
      const auto& wayId = std::get<1>(way);
      const auto& wayGeom = std::get<2>(way);
      const auto& wayNodeIds = std::get<3>(way);

      // Check if our "area" has successors in the DAG, if yes we are part of
      // the DAG and don't need to calculate any relation again.
      if (!_directedAreaGraph.findSuccessorsFast(wayId * 2).empty()) {
        skippedInDAG++;
        continue;
      }

      std::string wayIRI = _writer->generateIRI(
          osm2ttl::ttl::constants::NAMESPACE__OSM_WAY, wayId);

      // Set containing all areas we are inside of
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skipNodeContained;
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skipIntersects;
      std::set<osm2ttl::util::DirectedGraph<osm2ttl::osm::Area::id_t>::entry_t>
          skipContains;

      // Store known areas in set.
      for (const auto& nodeId : wayNodeIds) {
        auto nodeDataIt = nodeData.find(nodeId);
        if (nodeDataIt == nodeData.end()) {
          continue;
        }
        for (const auto& areaId : nodeDataIt->second) {
          skipNodeContained.insert(areaId);
        }
      }

      std::vector<SpatialAreaValue> queryResult;
      _spatialIndex.query(boost::geometry::index::intersects(wayEnvelope),
                          std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
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
        areas++;

        bool doesIntersect = false;
        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);
        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          intersectsSkippedByDAG++;
          doesIntersect = true;
        } else if (skipNodeContained.find(areaId) != skipNodeContained.end()) {
          intersectsByNodeInfo++;
          doesIntersect = true;

          for (const auto& newSkip :
               _directedAreaGraph.findSuccessorsFast(areaId)) {
            skipIntersects.insert(newSkip);
          }
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, wayIRI);
          if (_config.addInverseRelationDirection) {
            _writer->writeTriple(
                wayIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                areaIRI);
          }
        } else {
          intersectsChecks++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto start = std::chrono::steady_clock::now();
#endif
          doesIntersect = boost::geometry::intersects(wayGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Way", "doesIntersect", areaId, "area", wayId, "way",
                std::chrono::nanoseconds(end - start), doesIntersect));
          }
#endif
          if (!doesIntersect) {
            continue;
          }
          intersectsOk++;

          for (const auto& newSkip :
               _directedAreaGraph.findSuccessorsFast(areaId)) {
            skipIntersects.insert(newSkip);
          }
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTS, wayIRI);
          if (_config.addInverseRelationDirection) {
            _writer->writeTriple(
                wayIRI, osm2ttl::ttl::constants::IRI__OGC_INTERSECTED_BY,
                areaIRI);
          }
        }
        if (!doesIntersect) {
          continue;
        }
        if (skipContains.find(areaId) != skipContains.end()) {
          containsSkippedByDAG++;
        } else {
          containsChecks++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          start = std::chrono::steady_clock::now();
#endif
          bool isCoveredByEnvelope =
              boost::geometry::covered_by(wayEnvelope, areaEnvelope);
#ifdef ENABLE_GEOMETRY_STATISTIC
          end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Way", "isCoveredByEnvelope", areaId, "area", wayId,
                "way", std::chrono::nanoseconds(end - start),
                isCoveredByEnvelope));
          }
#endif
          if (!isCoveredByEnvelope) {
            continue;
          }
          containsOkEnvelope++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          start = std::chrono::steady_clock::now();
#endif
          bool isCoveredBy = boost::geometry::covered_by(wayGeom, areaGeom);
#ifdef ENABLE_GEOMETRY_STATISTIC
          end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Way", "isCoveredBy2", areaId, "area", wayId, "way",
                std::chrono::nanoseconds(end - start), isCoveredBy));
          }
#endif
          if (!isCoveredBy) {
            continue;
          }
          containsOk++;

          for (const auto& newSkip :
               _directedAreaGraph.findSuccessorsFast(areaId)) {
            skipContains.insert(newSkip);
          }
          _writer->writeTriple(
              areaIRI, osm2ttl::ttl::constants::IRI__OGC_CONTAINS, wayIRI);
          if (_config.addInverseRelationDirection) {
            _writer->writeTriple(wayIRI,
                                 osm2ttl::ttl::constants::IRI__OGC_CONTAINED_BY,
                                 areaIRI);
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2ttl::util::currentTimeFormatted() << " "
              << "... done with looking at " << areas << " areas" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << intersectsChecks
              << " intersection checks performed, " << intersectsSkippedByDAG
              << " skipped by DAG, " << intersectsByNodeInfo
              << " skipped by NodeInfo" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << "intersect: " << intersectsChecks << " yes: " << intersectsOk
              << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << containsChecks
              << " contains checks performed, " << containsSkippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " "
              << "contains: " << containsChecks
              << " contains envelope: " << containsOkEnvelope
              << " yes: " << containsOk << std::endl;
    std::cerr << osm2ttl::util::formattedTimeSpacer << " " << skippedInDAG
              << " ways are areas in the DAG -> skipped calculations for them"
              << std::endl;
  }
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;

#pragma clang diagnostic pop
