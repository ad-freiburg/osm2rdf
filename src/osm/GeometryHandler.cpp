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

#include "osm2rdf/osm/GeometryHandler.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "boost/archive/binary_iarchive.hpp"
#include "boost/geometry.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/thread.hpp"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/ttl/Constants.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/DirectedAcyclicGraph.h"
#include "osm2rdf/util/DirectedGraph.h"
#include "osm2rdf/util/ProgressBar.h"
#include "osm2rdf/util/Time.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-loop-convert"

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::GeometryHandler<W>::GeometryHandler(
    const osm2rdf::config::Config& config, osm2rdf::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer),
      _statistics(config, config.geomStatisticsPath.string()),
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
osm2rdf::osm::GeometryHandler<W>::~GeometryHandler() = default;

// ____________________________________________________________________________
template <typename W>
std::string osm2rdf::osm::GeometryHandler<W>::statisticLine(
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
void osm2rdf::osm::GeometryHandler<W>::area(const osm2rdf::osm::Area& area) {
  auto geom = area.geom();

  // inner simplified geom, empty by default
  auto innerGeom = osm2rdf::geometry::Area();

  // outer simplified geom, empty by default
  auto outerGeom = osm2rdf::geometry::Area();

  if (_config.simplifyGeometries > 0) {
    geom = simplifyGeometry(geom);
  }

  if (!_config.dontUseInnerOuterGeoms && (area.hasName() || !area.fromWay())) {
    innerGeom = simplifiedArea(geom, true);
    outerGeom = simplifiedArea(geom, false);
  }

#pragma omp critical(areaDataInsert)
  {
    if (area.hasName()) {
      if (_config.minimalAreaEnvelopeRatio <= 0.0 ||
          area.geomArea() / area.envelopeArea() >=
              _config.minimalAreaEnvelopeRatio) {
        _spatialStorageArea.push_back(SpatialAreaValue(
            area.envelope(), area.id(), geom, area.objId(), area.geomArea(),
            area.fromWay(), innerGeom, outerGeom));
      } else {
        // we have bad area envelope proportions -> treat as unnamed area
        _oaUnnamedAreas << SpatialAreaValue(
            area.envelope(), area.id(), geom, area.objId(), area.geomArea(),
            area.fromWay(), innerGeom, outerGeom);
        _numUnnamedAreas++;
      }
    } else if (!area.fromWay()) {
      // Areas from ways are handled in GeometryHandler<W>::way
      _oaUnnamedAreas << SpatialAreaValue(area.envelope(), area.id(), geom,
                                          area.objId(), area.geomArea(),
                                          area.fromWay(), innerGeom, outerGeom);
      _numUnnamedAreas++;
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::node(const osm2rdf::osm::Node& node) {
#pragma omp critical(nodeDataInsert)
  {
    _oaNodes << SpatialNodeValue(node.envelope(), node.id(), node.geom());
    _numNodes++;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::way(const osm2rdf::osm::Way& way) {
  std::vector<uint64_t> nodeIds;
  nodeIds.reserve(way.nodes().size());
  for (const auto& nodeRef : way.nodes()) {
    nodeIds.push_back(nodeRef.id());
  }
  auto geom = way.geom();
  if (_config.simplifyGeometries > 0) {
    geom = simplifyGeometry(geom);
  }
#pragma omp critical(wayDataInsert)
  {
    _oaWays << SpatialWayValue(way.envelope(), way.id(), geom, nodeIds);
    _numWays++;
  }
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
G osm2rdf::osm::GeometryHandler<W>::simplifyGeometry(const G& g) {
  G geom;
  auto perimeter_or_length =
      std::max(boost::geometry::perimeter(g), boost::geometry::length(g));
  do {
    boost::geometry::simplify(
        g, geom,
        osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR *
            perimeter_or_length * _config.simplifyGeometries);
    perimeter_or_length /= 2;
  } while (
      (boost::geometry::is_empty(geom) || !boost::geometry::is_valid(geom)) &&
      perimeter_or_length >=
          osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR);
  if (!boost::geometry::is_valid(geom)) {
    return g;
  }
  if (boost::geometry::is_empty(geom)) {
    return g;
  }
  return geom;
}

// ____________________________________________________________________________
template <typename W>
template <int MODE>
bool osm2rdf::osm::GeometryHandler<W>::ioDouglasPeucker(
    const boost::geometry::model::ring<osm2rdf::geometry::Location>&
        inputPoints,
    boost::geometry::model::ring<osm2rdf::geometry::Location>& outputPoints,
    size_t l, size_t r, double eps) const {
  // this is basically a verbatim translation from Hannah's qlever map UI code

  assert(r >= l);
  assert(inputPoints.size());
  assert(r < inputPoints.size());

  if (l == r) {
    outputPoints.push_back(inputPoints[l]);
    return false;
  }

  if (l + 1 == r) {
    outputPoints.push_back(inputPoints[l]);
    outputPoints.push_back(inputPoints[r]);
    return false;
  }
  // Compute the position of the point m between l and r that is furthest aways
  // from the line segment connecting l and r. Note that l < m < r, that is, it
  // cannot (and must not) happen that m == l or m == r.
  //
  // NOTE: If all points happen to lie directly on the line segment, max_dist ==
  // 0 and we can simplify without loss, no matter which variant.

  size_t m;
  // double max_dist = -1;
  auto m_left = l;
  auto m_right = l;
  double max_dist_left = 0;
  double max_dist_right = 0;
  auto L = inputPoints[l];
  auto R = inputPoints[r];

  // L and R should be different points.
  if (L.get<0>() == R.get<0>() && L.get<1>() == R.get<1>()) {
    std::cerr << "DOUGLAS PEUCKER FAIL!" << std::endl;
    // TODO: handle
    return false;
  }
  // Compute point furthest to the left (negative value for
  // distanceFromPointToLine) and furthest to the right (positive value).
  for (auto k = l + 1; k <= r - 1; k++) {
    auto dist = signedDistanceFromPointToLine(L, R, inputPoints[k]);
    if (dist < 0 && -dist > max_dist_left) {
      m_left = k;
      max_dist_left = -dist;
    }
    if (dist > 0 && dist > max_dist_right) {
      m_right = k;
      max_dist_right = dist;
    }
  }

  bool simplify = false;

  // INNER Douglas-Peucker: Simplify iff there is no point to the *left* and the
  // rightmost point has distance <= eps. Otherwise m is the leftmost point or,
  // if there is no such point, the rightmost point.
  if (MODE == 2) {
    simplify = (max_dist_left == 0 && max_dist_right <= eps);
    m = max_dist_left > 0 ? m_left : m_right;
  }

  // OUTER Douglas-Peucker: Simplify iff there is no point to the *right*
  // *and* the leftmost point has distance <= eps. Otherwise m is the rightmost
  // point or if there is no such point the leftmost point.
  if (MODE == 3) {
    simplify = (max_dist_right == 0 && max_dist_left <= eps);
    m = max_dist_right > 0 ? m_right : m_left;
  }

  // Simplification case: If m is at most eps away from the line segment
  // conecting l and r, we can simplify the part of the polygon from l to r by
  // the line segment that connects l and r.
  // assert(m > l);
  // assert(m < r);
  if (simplify) {
    outputPoints.push_back(L);
    outputPoints.push_back(R);
    return true;
  }

  // Recursion case: If we come here, we have a point at position m, where l < m
  // < r and that point is more than eps away from the line segment connecting l
  // and r. Then we call the algorithm recursively on the part to the left of m
  // and the part to the right of m. NOTE: It's a matter of taste whether we
  // include m in the left recursion or the right recursion, but we should not
  // include it in both.
  bool a = ioDouglasPeucker<MODE>(inputPoints, outputPoints, l, m, eps);
  bool b = ioDouglasPeucker<MODE>(inputPoints, outputPoints, m + 1, r, eps);

  return a || b;
}

// ____________________________________________________________________________
template <typename W>
double osm2rdf::osm::GeometryHandler<W>::signedDistanceFromPointToLine(
    const osm2rdf::geometry::Location& A, const osm2rdf::geometry::Location& B,
    const osm2rdf::geometry::Location& C) {
  // Check that the input is OK and not A == B.
  if (A.get<0>() == B.get<0>() && A.get<1>() == B.get<1>()) {
    return 0;
  }
  // The actual computation, see this Wikipedia article for the formula:
  // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
  double distAB = sqrt((A.get<0>() - B.get<0>()) * (A.get<0>() - B.get<0>()) +
                       (A.get<1>() - B.get<1>()) * (A.get<1>() - B.get<1>()));
  double areaTriangleTimesTwo =
      (B.get<1>() - A.get<1>()) * (A.get<0>() - C.get<0>()) -
      (B.get<0>() - A.get<0>()) * (A.get<1>() - C.get<1>());
  return areaTriangleTimesTwo / distAB;
}

// ____________________________________________________________________________
template <typename W>
osm2rdf::geometry::Area osm2rdf::osm::GeometryHandler<W>::simplifiedArea(
    const osm2rdf::geometry::Area& g, bool inner) const {
  // default value: empty area, means no inner geom
  osm2rdf::geometry::Area ret;

  // skip trivial / erroneous geoms
  if (!boost::geometry::is_valid(g)) {
    return ret;
  }

  if (boost::geometry::is_empty(g)) {
    return ret;
  }

  auto periOrLength =
      std::max(boost::geometry::perimeter(g), boost::geometry::length(g));
  double eps = periOrLength * _config.simplifyGeometriesInnerOuter;

  size_t numPointsOld = 0;
  size_t numPointsNew = 0;

  for (const auto& poly : g) {
    osm2rdf::geometry::Polygon simplified;
    numPointsOld += poly.outer().size();

    for (const auto& origInner : poly.inners()) {
      numPointsOld += origInner.size();
      if (origInner.size() < 4) {
        numPointsNew += origInner.size();
        simplified.inners().push_back(origInner);
        continue;
      }

      // inner polygons are given in counter-clockswise order
      // assert(polygonOrientation(origInner) != -1);

      // simplify the inner geometries with outer simplification, because
      // inner geometries are given counter-clockwise, it is not
      // necessary to swap the simplification mode
      boost::geometry::model::ring<osm2rdf::geometry::Location> retDP;
      size_t m = floor(origInner.size() / 2);
      if (inner) {
        ioDouglasPeucker<2>(origInner, retDP, 0, m, eps);
        ioDouglasPeucker<2>(origInner, retDP, m + 1, origInner.size() - 1, eps);
      } else {
        ioDouglasPeucker<3>(origInner, retDP, 0, m, eps);
        ioDouglasPeucker<3>(origInner, retDP, m + 1, origInner.size() - 1, eps);
      }
      retDP.push_back(retDP.front());  // ensure valid polyon
      simplified.inners().push_back(retDP);
      numPointsNew += retDP.size();
    }

    if (poly.outer().size() < 4) {
      numPointsNew += poly.outer().size();
      simplified.outer() = poly.outer();
    } else {
      // assert(polygonOrientation(poly.outer()) != 1);

      // simplify the outer geometry with inner simplification
      boost::geometry::model::ring<osm2rdf::geometry::Location> retDP;
      size_t m = floor(poly.outer().size() / 2);
      if (inner) {
        ioDouglasPeucker<2>(poly.outer(), retDP, 0, m, eps);
        ioDouglasPeucker<2>(poly.outer(), retDP, m + 1, poly.outer().size() - 1,
                            eps);
      } else {
        ioDouglasPeucker<3>(poly.outer(), retDP, 0, m, eps);
        ioDouglasPeucker<3>(poly.outer(), retDP, m + 1, poly.outer().size() - 1,
                            eps);
      }
      retDP.push_back(retDP.front());  // ensure valid polyon
      numPointsNew += retDP.size();
      simplified.outer() = retDP;
    }

    ret.push_back(simplified);
  }

  if (numPointsNew == numPointsOld) {
    // nothing changed, return empty poly to avoid extra space and double
    // checking later on
    return osm2rdf::geometry::Area();
  }

  return ret;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::closeExternalStorage() {
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
void osm2rdf::osm::GeometryHandler<W>::calculateRelations() {
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
    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Closing statistics files..." << std::endl;
    _statistics.close("[\n", "{}\n]");
    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareRTree() {
  std::cerr << osm2rdf::util::currentTimeFormatted()
            << " Packing area r-tree with " << _spatialStorageArea.size()
            << " entries ... " << std::endl;
  _spatialIndex =
      SpatialIndex(_spatialStorageArea.begin(), _spatialStorageArea.end());
  std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done"
            << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareDAG() {
  // Store dag
  osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t> tmpDirectedAreaGraph;
  {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " Sorting "
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
    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done "
              << std::endl;

    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Generating non-reduced DAG from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osm2rdf::util::ProgressBar progressBar{_spatialStorageArea.size(), true};
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
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
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
        bool isCoveredBy = areaInArea(entry, area);

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
        const auto& successors = tmpDirectedAreaGraph.findSuccessors(entryId);
        skip.insert(successors.begin(), successors.end());
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done with "
              << checks << " checks, " << skippedBySize << " skipped by Size,"
              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << skippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << (checks - (skippedByDAG + skippedBySize))
              << " checks performed" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " contains: " << contains
              << " yes: " << containsOk << std::endl;
  }
  if (_config.writeDAGDotFiles) {
    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Dumping non-reduced DAG as " << _config.output
              << ".non-reduced.dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".non-reduced.dot";
    tmpDirectedAreaGraph.dump(p);
    std::cerr << osm2rdf::util::currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " Reducing DAG with "
              << tmpDirectedAreaGraph.getNumEdges() << " edges and "
              << tmpDirectedAreaGraph.getNumVertices() << " vertices ... "
              << std::endl;

    // Prepare non-reduced DAG for cleanup
    tmpDirectedAreaGraph.prepareFindSuccessorsFast();
    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " ... fast lookup prepared ... " << std::endl;

    _directedAreaGraph = osm2rdf::util::reduceDAG(tmpDirectedAreaGraph, true);

    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " ... done, resulting in DAG with "
              << _directedAreaGraph.getNumEdges() << " edges and "
              << _directedAreaGraph.getNumVertices() << " vertices"
              << std::endl;
  }
  if (_config.writeDAGDotFiles) {
    std::cerr << osm2rdf::util::currentTimeFormatted() << " Dumping DAG as "
              << _config.output << ".dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".dot";
    _directedAreaGraph.dump(p);
    std::cerr << osm2rdf::util::currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Preparing fast above lookup in DAG ..." << std::endl;
    _directedAreaGraph.prepareFindSuccessorsFast();
    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::dumpNamedAreaRelations() {
  std::cerr << std::endl;
  std::cerr << osm2rdf::util::currentTimeFormatted()
            << " Dumping relations from DAG with "
            << _directedAreaGraph.getNumEdges() << " edges and "
            << _directedAreaGraph.getNumVertices() << " vertices ... "
            << std::endl;

  osm2rdf::util::ProgressBar progressBar{_directedAreaGraph.getNumVertices(),
                                         true};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  std::vector<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
      vertices = _directedAreaGraph.getVertices();
#pragma omp parallel for shared(                                            \
    vertices, osm2rdf::ttl::constants::NAMESPACE__OSM_WAY,                  \
    osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,                       \
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_AREA,                    \
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,                \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA,                  \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, progressBar, \
    entryCount) default(none) schedule(static)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto id = vertices[i];
    const auto& entry = _spatialStorageArea[_spatialStorageAreaIndex[id]];
    const auto& entryObjId = std::get<3>(entry);
    const auto& entryFromWay = std::get<5>(entry);
    std::string entryIRI = _writer->generateIRI(
        entryFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                     : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
        entryObjId);
    for (const auto& dst : _directedAreaGraph.getEdges(id)) {
      assert(_spatialStorageAreaIndex[dst] < _spatialStorageArea.size());
      const auto& area = _spatialStorageArea[_spatialStorageAreaIndex[dst]];
      const auto& areaObjId = std::get<3>(area);
      const auto& areaFromWay = std::get<5>(area);
      std::string areaIRI = _writer->generateIRI(
          areaFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                      : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
          areaObjId);

      _writer->writeTriple(areaIRI,
                           osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_AREA,
                           entryIRI);
      _writer->writeTriple(
          areaIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA,
          entryIRI);
    }
#pragma omp critical(progress)
    progressBar.update(entryCount++);
  }

  progressBar.done();

  std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done"
            << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::dumpUnnamedAreaRelations() {
  if (_config.noAreaGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Skipping contains relation for unnamed areas ... disabled"
              << std::endl;
  } else if (_numUnnamedAreas == 0) {
    std::cerr << std::endl;
    std::cerr
        << osm2rdf::util::currentTimeFormatted() << " "
        << "Skipping contains relation for unnamed areas ... no unnamed area"
        << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Contains relations for " << _numUnnamedAreas
              << " unnamed areas in " << _spatialIndex.size() << " areas ..."
              << std::endl;
    std::ifstream ifs(_config.getTempPath("spatial", "areas_unnamed"),
                      std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);

    osm2rdf::util::ProgressBar progressBar{_numUnnamedAreas, true};
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
#pragma omp parallel for shared( \
    osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA, \
    progressBar, entryCount, ia) reduction(+:areas,intersectsSkippedByDAG, \
    containsSkippedByDAG, intersectsChecks, intersectsOk, containsChecks, \
    containsOk, containsOkEnvelope)      \
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
          entryFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                       : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
          entryObjId);

      // Set containing all areas we are inside of
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skipIntersects;
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
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
            areaFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
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

          const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          _writer->writeTriple(
              areaIRI,
              osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,
              entryIRI);
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
          bool isCoveredBy = areaInArea(entry, area);

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

          const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
          skipContains.insert(successors.begin(), successors.end());

          _writer->writeTriple(
              areaIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,
              entryIRI);
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "... done with looking at " << areas << " areas" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << intersectsChecks
              << " intersection checks performed, " << intersectsSkippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << "intersect: " << intersectsChecks << " yes: " << intersectsOk
              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << containsChecks
              << " contains checks performed, " << containsSkippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << "contains: " << containsChecks
              << " contains envelope: " << containsOkEnvelope
              << " yes: " << containsOk << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::NodesContainedInAreasData
osm2rdf::osm::GeometryHandler<W>::dumpNodeRelations() {
  // Store for each node all relevant areas
  NodesContainedInAreasData nodeData;
  if (_config.noNodeGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Skipping contains relation for nodes ... disabled"
              << std::endl;
  } else if (_numNodes == 0) {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Skipping contains relation for nodes ... no nodes"
              << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Contains relations for " << _numNodes << " nodes in "
              << _spatialIndex.size() << " areas ..." << std::endl;

    std::ifstream ifs(_config.getTempPath("spatial", "nodes"),
                      std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);

    osm2rdf::util::ProgressBar progressBar{_numNodes, true};
    size_t entryCount = 0;
    size_t checks = 0;
    size_t contains = 0;
    size_t containsOk = 0;
    size_t skippedByDAG = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared( \
    std::cout, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,\
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, nodeData, \
    progressBar, entryCount, ia) reduction(+:checks, \
    skippedByDAG, contains, containsOk) default(none) schedule(dynamic)
    for (size_t i = 0; i < _numNodes; i++) {
      SpatialNodeValue node;
#pragma omp critical(loadEntry)
      ia >> node;
      const auto& nodeEnvelope = std::get<0>(node);
      const auto& nodeId = std::get<1>(node);
      std::string nodeIRI = _writer->generateIRI(
          osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, nodeId);

      // Set containing all areas we are inside of
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
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

        bool isCoveredBy = nodeInArea(node, area);

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

        const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
        skip.insert(successors.begin(), successors.end());

        std::string areaIRI = _writer->generateIRI(
            areaFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                        : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
            areaObjId);
        _writer->writeTriple(
            areaIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,
            nodeIRI);
        _writer->writeTriple(
            areaIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,
            nodeIRI);
      }
#pragma omp critical(nodeDataChange)
      std::copy(skip.begin(), skip.end(), std::back_inserter(nodeData[nodeId]));
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "... done with looking at " << checks << " areas, "
              << skippedByDAG << " skipped by DAG" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << contains
              << " checks performed" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << "contains: " << contains << " yes: " << containsOk
              << std::endl;
  }
  return nodeData;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::dumpWayRelations(
    const osm2rdf::osm::NodesContainedInAreasData& nodeData) {
  if (_config.noWayGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Skipping contains relation for ways ... disabled"
              << std::endl;
  } else if (_numWays == 0) {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Skipping contains relation for ways ... no ways" << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "Contains relations for " << _numWays << " ways in "
              << _spatialIndex.size() << " areas ..." << std::endl;

    std::ifstream ifs(_config.getTempPath("spatial", "ways"), std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);

    osm2rdf::util::ProgressBar progressBar{_numWays, true};
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
#pragma omp parallel for shared( \
    std::cout, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, nodeData, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA, \
    progressBar, entryCount, ia) reduction(+:areas,intersectsSkippedByDAG, containsSkippedByDAG, skippedInDAG, \
    intersectsByNodeInfo, intersectsChecks, intersectsOk, containsChecks, containsOk, containsOkEnvelope)    \
    default(none) schedule(dynamic)

    for (size_t i = 0; i < _numWays; i++) {
      SpatialWayValue way;
#pragma omp critical(loadEntry)
      ia >> way;
      const auto& wayEnvelope = std::get<0>(way);
      const auto& wayId = std::get<1>(way);
      const auto& wayNodeIds = std::get<3>(way);

      // Check if our "area" has successors in the DAG, if yes we are part of
      // the DAG and don't need to calculate any relation again.
      if (!_directedAreaGraph.findSuccessorsFast(wayId * 2).empty()) {
        skippedInDAG++;
        continue;
      }

      std::string wayIRI = _writer->generateIRI(
          osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, wayId);

      // Set containing all areas we are inside of
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skipNodeContained;
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skipIntersects;
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
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
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromWay = std::get<5>(area);

        if (areaFromWay && areaObjId == wayId) {
          continue;
        }
        areas++;

        bool doesIntersect = false;
        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          intersectsSkippedByDAG++;
          doesIntersect = true;
        } else if (skipNodeContained.find(areaId) != skipNodeContained.end()) {
          intersectsByNodeInfo++;
          doesIntersect = true;

          const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
              areaObjId);

          _writer->writeTriple(
              areaIRI,
              osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,
              wayIRI);
        } else {
          intersectsChecks++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto start = std::chrono::steady_clock::now();
#endif
          doesIntersect = wayIntersectsArea(way, area);
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

          const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
              areaObjId);

          _writer->writeTriple(
              areaIRI,
              osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,
              wayIRI);
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

          bool isCoveredBy = wayInArea(way, area);

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

          const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
          skipContains.insert(successors.begin(), successors.end());

          std::string areaIRI = _writer->generateIRI(
              areaFromWay ? osm2rdf::ttl::constants::NAMESPACE__OSM_WAY
                          : osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,
              areaObjId);

          _writer->writeTriple(
              areaIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,
              wayIRI);
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " "
              << "... done with looking at " << areas << " areas" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << intersectsChecks
              << " intersection checks performed, " << intersectsSkippedByDAG
              << " skipped by DAG, " << intersectsByNodeInfo
              << " skipped by NodeInfo" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << "intersect: " << intersectsChecks << " yes: " << intersectsOk
              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << containsChecks
              << " contains checks performed, " << containsSkippedByDAG
              << " skipped by DAG" << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << "contains: " << containsChecks
              << " contains envelope: " << containsOkEnvelope
              << " yes: " << containsOk << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << skippedInDAG
              << " ways are areas in the DAG -> skipped calculations for them"
              << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::nodeInArea(
    const SpatialNodeValue& a, const SpatialAreaValue& b) const {
  const auto& geomA = std::get<2>(a);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    return boost::geometry::covered_by(geomA, geomB);
  }

  if (boost::geometry::covered_by(geomA, innerGeomB)) {
    // if covered by simplified inner, we are definitely contained
    return true;
  } else if (!boost::geometry::covered_by(geomA, outerGeomB)) {
    // if NOT covered by simplified out, we are definitely not contained
    return false;
  } else if (boost::geometry::covered_by(geomA, geomB)) {
    return true;
  }

  return false;
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::wayIntersectsArea(
    const SpatialWayValue& a, const SpatialAreaValue& b) const {
  const auto& geomA = std::get<2>(a);

  const auto& geomB = std::get<2>(b);
  const auto& envelopeB = std::get<0>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    return boost::geometry::intersects(geomA, geomB);
  }

  if (!boost::geometry::intersects(geomA, envelopeB)) {
    // if does not intersect with envelope, we definitely dont intersect
    return false;
  } else if (boost::geometry::intersects(geomA, innerGeomB)) {
    // if intersects simplified inner, we definitely intersect
    return true;
  } else if (!boost::geometry::intersects(geomA, outerGeomB)) {
    // if NOT intersecting simplified outer, we are definitely NOT
    // intersecting
    return false;
    // } else if (boost::geometry::covered_by(wayEnvelope, areaGeom)) {
    // isCoveredBy= true;
  } else if (boost::geometry::intersects(geomA, geomB)) {
    return true;
  }

  return false;
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::wayInArea(
    const SpatialWayValue& a, const SpatialAreaValue& b) const {
  const auto& geomA = std::get<2>(a);
  const auto& envelopeA = std::get<0>(a);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    return boost::geometry::covered_by(geomA, geomB);
  }

  boost::geometry::model::ring<osm2rdf::geometry::Location> ring;
  ring.push_back(envelopeA.min_corner());
  ring.push_back({envelopeA.min_corner().get<0>(), envelopeA.max_corner().get<1>()});
  ring.push_back(envelopeA.max_corner());
  ring.push_back({envelopeA.max_corner().get<0>(), envelopeA.min_corner().get<1>()});
  ring.push_back(envelopeA.min_corner());
  osm2rdf::geometry::Polygon bboxPoly;
  bboxPoly.inners().push_back(ring);

  if (boost::geometry::covered_by(bboxPoly, innerGeomB)) {
    // if envelope covered by simplified inner, we are definitely
    // contained
    return true;
  } else if (boost::geometry::covered_by(geomA, innerGeomB)) {
    // if covered by simplified inner, we are definitely contained
    return true;
  } else if (!boost::geometry::covered_by(geomA, outerGeomB)) {
    // if NOT covered by simplified outer, we are definitely NOT
    // contained
    return false;
  } else if (boost::geometry::covered_by(geomA, geomB)) {
    return true;
  }

  return false;
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::areaInArea(
    const SpatialAreaValue& a, const SpatialAreaValue& b) const {
  const auto& geomA = std::get<2>(a);
  const auto& innerGeomA = std::get<6>(a);
  const auto& outerGeomA = std::get<7>(a);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB) ||
      boost::geometry::is_empty(outerGeomA) ||
      boost::geometry::is_empty(innerGeomA)) {
    return boost::geometry::covered_by(geomA, geomB);
  }

  if (boost::geometry::covered_by(outerGeomA, innerGeomB)) {
    // if simplified outer is covered by simplified inner, we are
    // definitely contained
    // assert(boost::geometry::covered_by(geomA, geomB));
    return true;
  } else if (!boost::geometry::covered_by(innerGeomA, outerGeomB)) {
    // if simplified inner is not covered by simplified outer, we are
    // definitely not contained
    // assert(!boost::geometry::covered_by(geomA, geomB));
    return false;
    // } else if (boost::geometry::covered_by(entryGeom, areaInnerGeom)) {
    // // if covered by simplified inner, we are definitely contained
    // isCoveredBy = true;
    // } else if (!boost::geometry::covered_by(entryGeom, areaOuterGeom))
    // {
    // // if NOT covered by simplified out, we are definitely not
    // contained isCoveredBy = false;
  } else if (boost::geometry::covered_by(geomA, geomB)) {
    return true;
  }

  return false;
}

// ____________________________________________________________________________
template <typename W>
int osm2rdf::osm::GeometryHandler<W>::polygonOrientation(
    const boost::geometry::model::ring<osm2rdf::geometry::Location>& polygon) {
  // https://de.wikipedia.org/wiki/Gau%C3%9Fsche_Trapezformel
  double sum = 0;
  for (size_t i = 0; i + 1 < polygon.size(); i++) {
    auto a = polygon[i];
    auto b = polygon[i + 1];
    sum += (a.get<0>() - b.get<0>()) * (a.get<1>() + b.get<1>());
  }

  return sum < 0 ? -1 : sum > 0 ? 1 : 0;
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER>;

#pragma clang diagnostic pop
