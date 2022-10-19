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
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/GeometryHandler.h"
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
      _containsStatistics(config, config.containsStatisticsPath.string()),
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
void osm2rdf::osm::GeometryHandler<W>::relation(
    const osm2rdf::osm::Relation& rel) {
  // careful, tags is returned via copy (why?)
  const auto& tags = rel.tags();
  auto i = tags.find("type");
  if (i != tags.end() && (i->second == "boundary" || i->second == "multipolygon")) {
    bool inserted = false;

    for (const auto& m : rel.members()) {
      if (m.type() == osm2rdf::osm::RelationMemberType::WAY &&
          (m.role() == "outer" || m.role() == "inner")) {
#pragma omp critical
        {
          _areaBorderWaysIndex[m.id()].push_back(rel.id());
        }
        inserted = true;
      }
    }

    if (inserted) {
#pragma omp critical
      {
        std::sort(_areaBorderWaysIndex[m.id()].begin(),
                  _areaBorderWaysIndex[m.id()].end());
      }
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::area(const osm2rdf::osm::Area& area) {
  // inner simplified geom, empty by default
  auto innerGeom = osm2rdf::geometry::Area();

  // outer simplified geom, empty by default
  auto outerGeom = osm2rdf::geometry::Area();

  osm2rdf::geometry::Area geom = area.geom();

  if (_config.simplifyGeometries > 0) {
    geom = simplifyGeometry(area.geom());
  }

  if (!_config.dontUseInnerOuterGeoms && (area.hasName() || !area.fromWay())) {
    innerGeom = simplifiedArea(geom, true);
    outerGeom = simplifiedArea(geom, false);
  }

  std::vector<osm2rdf::geometry::Box> envelopes;
  envelopes.push_back(area.envelope());
  if (area.geom().size() > 1) {
    for (const auto& p : area.geom()) {
      osm2rdf::geometry::Box box;
      boost::geometry::envelope(p, box);
      envelopes.push_back(box);
    }
  } else {
    envelopes.push_back(area.envelope());
  }

  const auto& boxIds =
      pack(getBoxIds(area.geom(), area.envelope(), innerGeom, outerGeom));

#pragma omp critical(areaDataInsert)
  {
    if (area.hasName()) {
      if (_config.minimalAreaEnvelopeRatio <= 0.0 ||
          area.geomArea() / area.envelopeArea() >=
              _config.minimalAreaEnvelopeRatio) {
        _spatialStorageArea.push_back(SpatialAreaValue(
            envelopes, area.id(), geom, area.objId(), area.geomArea(),
            area.fromWay() ? 1 : 0, innerGeom, outerGeom, boxIds));
      } else {
        // we have bad area envelope proportions -> treat as unnamed area
        _oaUnnamedAreas << SpatialAreaValue(
            envelopes, area.id(), geom, area.objId(), area.geomArea(),
            area.fromWay() ? 1 : 0, innerGeom, outerGeom, boxIds);
        _numUnnamedAreas++;
      }
    } else if (!area.fromWay()) {
      // Areas from ways are handled in GeometryHandler<W>::way
      _oaUnnamedAreas << SpatialAreaValue(
          envelopes, area.id(), geom, area.objId(), area.geomArea(),
          area.fromWay() ? 1 : 0, innerGeom, outerGeom, boxIds);
      _numUnnamedAreas++;
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::node(const osm2rdf::osm::Node& node) {
#pragma omp critical(nodeDataInsert)
  {
    _oaNodes << SpatialNodeValue(node.id(), node.geom());
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
    std::cerr << "A" << std::endl;
  }

  std::vector<osm2rdf::geometry::Box> boxes;

  size_t CHUNKSIZE = 25;

  if (geom.size() >= 2 * CHUNKSIZE) {
    size_t i = 0;
    while (i < geom.size()) {
      osm2rdf::geometry::Box box;

      osm2rdf::geometry::Way tmp(
          geom.begin() + i,
          geom.begin() + std::min(i + CHUNKSIZE, geom.size()));

      boost::geometry::envelope(tmp, box);
      boxes.push_back(box);
      i += CHUNKSIZE;
    }
  } else {
    boxes.push_back(way.envelope());
  }

#pragma omp critical(wayDataInsert)
  {
    _oaWays << SpatialWayValue(way.envelope(), way.id(), geom, nodeIds, boxes,
                               pack(getBoxIds(geom, way.envelope())));
    _numWays++;
  }
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
G osm2rdf::osm::GeometryHandler<W>::simplifyGeometry(const G& g) const {
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

      double eps = sqrt(-boost::geometry::area(origInner) / 3.14) * 3.14 * 2 *
                   _config.simplifyGeometriesInnerOuter;

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
      double eps = sqrt(boost::geometry::area(poly.outer()) / 3.14) * 3.14 * 2 *
                   _config.simplifyGeometriesInnerOuter;

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

  if (numPointsNew >= numPointsOld) {
    // gain too low, return empty poly to avoid extra space and double
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
    _containsStatistics.open();
  }
  // prepareExplicitMPs();
  if (_config.minIntersectArea < std::numeric_limits<double>::infinity()) {
    prepareRTree();
    prepareDummyRegionsIntersect();
  }
  if (_config.dummyGridCellSize < std::numeric_limits<double>::infinity()) {
    prepareRTree();
    prepareDummyRegionsGrid();
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
    _containsStatistics.close();
    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done"
              << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareRTree() {
  // empty the rtree!
  _spatialIndex.clear();
  std::cerr << std::endl;
  std::cerr << osm2rdf::util::currentTimeFormatted() << " Sorting "
            << _spatialStorageArea.size() << " areas ... " << std::endl;
  std::sort(_spatialStorageArea.begin(), _spatialStorageArea.end(),
            [](const auto& a, const auto& b) {
              return std::get<4>(a) > std::get<4>(b);
            });
  std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done "
            << std::endl;

  std::cerr << osm2rdf::util::currentTimeFormatted()
            << " Packing area r-tree with " << _spatialStorageArea.size()
            << " entries ... " << std::endl;

  for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
    _spatialIndex.insert(
        SpatialAreaRefValue(std::get<0>(_spatialStorageArea[i])[0], i));
  }

  std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done"
            << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareExplicitMPs() {
  {
    // Prepare id based lookup table for later usage...
    _spatialStorageAreaIndex.reserve(_spatialStorageArea.size());
    for (size_t i = 0; i < _spatialStorageArea.size(); ++i) {
      const auto& area = _spatialStorageArea[i];
      _spatialStorageAreaIndex[std::get<1>(area)] = i;
    }

    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Generating explicit multipolygon regions from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osm2rdf::util::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;

    size_t dummyAreaCountBef = _dummyAreaCount;

    progressBar.update(entryCount);

    // we only iterate over all original elements - dummy areas will be added
    // to the end of _spatialStorageArea, and thus ignored for dummy area
    // creation
    std::vector<std::pair<osm2rdf::geometry::Area, double>> newDummyRegions;

#pragma omp parallel for shared(                                       \
    std::cout, std::cerr,                                              \
    osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_PARTITION,             \
    osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY, entryCount, \
    progressBar, newDummyRegions) default(none) schedule(dynamic)

    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryGeom = std::get<2>(entry);

      // if geom is a multigeometry, add all individual geometries as areas
      if (entryGeom.size() > 1) {
        for (const auto& poly : entryGeom) {
          osm2rdf::geometry::Area singlePolyMP{poly};
          double area = boost::geometry::area(singlePolyMP);

#pragma omp critical(addMPDummy)
          newDummyRegions.push_back({singlePolyMP, area});
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done, added "
              << (_dummyAreaCount - dummyAreaCountBef) << " new areas"
              << std::endl;

#pragma omp parallel for shared(newDummyRegions) default(none) schedule(dynamic)
    for (size_t i = 0; i < newDummyRegions.size(); i++) {
      addDummyRegion(newDummyRegions[i].first, newDummyRegions[i].second);
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareDummyRegionsGrid() {
  {
    // Prepare id based lookup table for later usage...
    _spatialStorageAreaIndex.reserve(_spatialStorageArea.size());
    for (size_t i = 0; i < _spatialStorageArea.size(); ++i) {
      const auto& area = _spatialStorageArea[i];
      _spatialStorageAreaIndex[std::get<1>(area)] = i;
    }

    size_t dummyAreaCountBef = _dummyAreaCount;

    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Generating dummy grid-based regions from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osm2rdf::util::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;

    progressBar.update(entryCount);

    double GRID_W = _config.dummyGridCellSize;
    double MAX_AREA = GRID_W * GRID_W;

    std::vector<std::pair<osm2rdf::geometry::Area, double>> newDummyRegions;

#pragma omp parallel for shared(                                       \
    std::cout, std::cerr, MAX_AREA, GRID_W, newDummyRegions,           \
    osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_PARTITION,             \
    osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY, entryCount, \
    progressBar) default(none) schedule(dynamic)

    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryEnvelopes = std::get<0>(entry);
      const auto& entryGeom = std::get<2>(entry);
      const auto& entryArea = std::get<4>(entry);

      if (entryArea <= MAX_AREA) continue;

      double boxW = entryEnvelopes[0].max_corner().get<0>() -
                    entryEnvelopes[0].min_corner().get<0>();
      double boxH = entryEnvelopes[0].max_corner().get<1>() -
                    entryEnvelopes[0].min_corner().get<1>();

      for (size_t x = 0; x < ceil(boxW / GRID_W); x++) {
        for (size_t y = 0; y < ceil(boxH / GRID_W); y++) {
          osm2rdf::geometry::Box box;
          box.min_corner().set<0>(entryEnvelopes[0].min_corner().get<0>() +
                                  x * GRID_W);
          box.min_corner().set<1>(entryEnvelopes[0].min_corner().get<1>() +
                                  y * GRID_W);
          box.max_corner().set<0>(entryEnvelopes[0].min_corner().get<0>() +
                                  (x + 1) * GRID_W);
          box.max_corner().set<1>(entryEnvelopes[0].min_corner().get<1>() +
                                  (y + 1) * GRID_W);

          osm2rdf::geometry::Area boxIntersect;
          boost::geometry::intersection(entryGeom, box, boxIntersect);
          double intersectArea = boost::geometry::area(boxIntersect);

          if (intersectArea > 0) {
#pragma omp critical(addGridDummy)
            newDummyRegions.push_back({boxIntersect, intersectArea});
          }
        }
      }

#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done, added "
              << newDummyRegions.size() << " dummy areas" << std::endl;

#pragma omp parallel for shared(newDummyRegions) default(none) schedule(dynamic)
    for (size_t i = 0; i < newDummyRegions.size(); i++) {
      addDummyRegion(newDummyRegions[i].first, newDummyRegions[i].second);
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareDummyRegionsIntersect() {
  {
    // Prepare id based lookup table for later usage...
    _spatialStorageAreaIndex.reserve(_spatialStorageArea.size());
    for (size_t i = 0; i < _spatialStorageArea.size(); ++i) {
      const auto& area = _spatialStorageArea[i];
      _spatialStorageAreaIndex[std::get<1>(area)] = i;
    }

    std::cerr << osm2rdf::util::currentTimeFormatted()
              << " Generating dummy regions from " << _spatialStorageArea.size()
              << " areas ... " << std::endl;

    osm2rdf::util::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;

    size_t dummyAreaCountBef = _dummyAreaCount;

    progressBar.update(entryCount);

    // we only iterate over all original elements - dummy areas will be added
    // to the end of _spatialStorageArea, and thus ignored for dummy area
    // creation
    double MIN_AREA = _config.minIntersectArea;

    std::vector<std::pair<osm2rdf::geometry::Area, double>> newDummyRegions;

#pragma omp parallel for shared(                                       \
    std::cout, std::cerr, MIN_AREA,                                    \
    osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_PARTITION,             \
    osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY, entryCount, \
    progressBar, newDummyRegions) default(none) schedule(dynamic)

    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryEnvelope = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryGeom = std::get<2>(entry);
      const auto& entryArea = std::get<4>(entry);

      // Set containing all areas we already checked
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skip;
      std::vector<SpatialAreaRefValue> queryResult;
      _spatialIndex.query(boost::geometry::index::intersects(entryEnvelope[0]),
                          std::back_inserter(queryResult));

      skip.insert(entryId);

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaArea = std::get<4>(area);

        if (areaId == entryId) {
          continue;
        }

        if (skip.find(areaId) != skip.end()) {
          continue;
        }

        // intersection will never be over the threshold, discard
        if (entryArea <= MIN_AREA && areaArea <= MIN_AREA) continue;

        // if only entryArea is too small, the intersection will still be too
        // small, but the difference might be over the threshold.
        // The best case scenario is then that the intersection is the entire
        // "entry", but if even in that case the resulting geometry covers too
        // much of "area", discard
        if (entryArea <= MIN_AREA && (areaArea - entryArea) / areaArea >= 0.5)
          continue;

        // same as above for entry
        if (areaArea <= MIN_AREA && (entryArea - areaArea) / areaArea >= 0.5)
          continue;

        // intersection
        osm2rdf::geometry::Area intersect;
        boost::geometry::intersection(entryGeom, areaGeom, intersect);
        double intersectArea = boost::geometry::area(intersect);
        double entryCovered = intersectArea / entryArea;
        double areaCovered = intersectArea / areaArea;

        if (intersectArea > MIN_AREA && entryCovered < 0.98 &&
            areaCovered < 0.98 && (entryCovered < 0.5 || areaCovered < 0.5)) {
#pragma omp critical(addDummy)
          newDummyRegions.push_back({intersect, intersectArea});
        }

        // difference in entry
        osm2rdf::geometry::Area diffEntry;
        boost::geometry::difference(entryGeom, intersect, diffEntry);
        double diffEntryArea = boost::geometry::area(diffEntry);
        double diffEntryEntryCovered = diffEntryArea / entryArea;
        double diffEntryAreaCovered = diffEntryArea / areaArea;
        if (diffEntryArea > MIN_AREA && diffEntryAreaCovered < 0.98 &&
            diffEntryEntryCovered < 0.98 &&
            (diffEntryAreaCovered < 0.5 || diffEntryEntryCovered < 0.5)) {
#pragma omp critical(addDummy)
          newDummyRegions.push_back({diffEntry, diffEntryArea});
        }

        // difference in area
        osm2rdf::geometry::Area diffArea;
        boost::geometry::difference(areaGeom, intersect, diffArea);
        double diffAreaArea = boost::geometry::area(diffArea);
        double diffAreaEntryCovered = diffAreaArea / entryArea;
        double diffAreaAreaCovered = diffAreaArea / areaArea;
        if (diffAreaArea > MIN_AREA && diffAreaEntryCovered < 0.98 &&
            diffAreaAreaCovered < 0.98 &&
            (diffAreaEntryCovered < 0.5 || diffAreaAreaCovered < 0.5)) {
#pragma omp critical(addDummy)
          newDummyRegions.push_back({diffArea, diffAreaArea});
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << osm2rdf::util::currentTimeFormatted() << " ... done, added "
              << newDummyRegions.size() << " dummy areas" << std::endl;

#pragma omp parallel for shared(newDummyRegions) default(none) schedule(dynamic)
    for (size_t i = 0; i < newDummyRegions.size(); i++) {
      addDummyRegion(newDummyRegions[i].first, newDummyRegions[i].second);
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::prepareDAG() {
  // Store dag
  osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t> tmpDirectedAreaGraph;
  {
    // Prepare id based lookup table for later usage...
    _spatialStorageAreaIndex.reserve(_spatialStorageArea.size());
    for (size_t i = 0; i < _spatialStorageArea.size(); ++i) {
      const auto& area = _spatialStorageArea[i];
      _spatialStorageAreaIndex[std::get<1>(area)] = i;
    }

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
    std::cout, std::cerr,\
                               osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_PARTITION, osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY,    entryCount, progressBar) reduction(+:checks, skippedBySize, skippedByDAG, \
    contains, containsOk) default(none) schedule(dynamic)

    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryEnvelopes = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryGeom = std::get<2>(entry);
      const auto& entryArea = std::get<4>(entry);

      // Set containing all areas we are inside of
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skip;
      std::vector<SpatialAreaRefValue> queryResult;
      _spatialIndex.query(boost::geometry::index::covers(entryEnvelopes[0]),
                          std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
                [this](const auto& a, const auto& b) {
                  return std::get<4>(_spatialStorageArea[a.second]) <
                         std::get<4>(_spatialStorageArea[b.second]);
                });

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaGeom = std::get<2>(area);
        const auto& areaArea = std::get<4>(area);

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
        // Approximate covered_by
        bool isCoveredBy = false;
        double intersectArea = 0;

        if (areaArea / entryArea > 0.95) {
          osm2rdf::geometry::Area intersect;
          boost::geometry::intersection(entryGeom, areaGeom, intersect);

          intersectArea = boost::geometry::area(intersect);

          isCoveredBy = fabs(1 - entryArea / intersectArea) < 0.05;
        }

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
        bool isEqual = isCoveredBy && fabs(1 - areaArea / intersectArea) < 0.05;
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
        {
          tmpDirectedAreaGraph.addEdge(entryId, areaId);
          const auto& successors = tmpDirectedAreaGraph.findSuccessors(entryId);
          skip.insert(successors.begin(), successors.end());
        }
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
    const auto& entryFromType = std::get<5>(entry);
    std::string entryIRI =
        _writer->generateIRI(areaNamespace(entryFromType), entryObjId);
    for (const auto& dst : _directedAreaGraph.getEdges(id)) {
      assert(_spatialStorageAreaIndex[dst] < _spatialStorageArea.size());
      const auto& area = _spatialStorageArea[_spatialStorageAreaIndex[dst]];
      const auto& areaObjId = std::get<3>(area);
      const auto& areaFromType = std::get<5>(area);
      std::string areaIRI =
          _writer->generateIRI(areaNamespace(areaFromType), areaObjId);

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
    progressBar.update(entryCount);
#pragma omp parallel for shared( \
    osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA, \
    progressBar, entryCount, ia) reduction(+:areas,intersectsSkippedByDAG, \
    containsSkippedByDAG, intersectsChecks, intersectsOk, containsChecks, \
    containsOk)      \
    default(none) schedule(dynamic)
    for (size_t i = 0; i < _numUnnamedAreas; i++) {
      SpatialAreaValue entry;
#pragma omp critical(loadEntry)
      ia >> entry;
      const auto& entryEnvelopes = std::get<0>(entry);
      const auto& entryId = std::get<1>(entry);
      const auto& entryObjId = std::get<3>(entry);
      const auto& entryFromType = std::get<5>(entry);
      std::string entryIRI =
          _writer->generateIRI(areaNamespace(entryFromType), entryObjId);

      // Set containing all areas we are inside of
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skipIntersects;
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skipContains;

      std::vector<SpatialAreaRefValue> queryResult;

      for (size_t i = 1; i < entryEnvelopes.size(); i++) {
        _spatialIndex.query(
            boost::geometry::index::intersects(entryEnvelopes[i]),
            std::back_inserter(queryResult));
      }

      if (entryEnvelopes.size() > 2) {
        // remove duplicates, may occur since we used multiple envelope queries
        // to build the result!
        std::sort(
            queryResult.begin(), queryResult.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        auto last = std::unique(
            queryResult.begin(), queryResult.end(),
            [](const auto& a, const auto& b) { return a.second == b.second; });

        // duplicates were swapped to  the end of vector, beginning at last
        queryResult.erase(last, queryResult.end());
      }

      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
                [this](const auto& a, const auto& b) {
                  return std::get<4>(_spatialStorageArea[a.second]) <
                         std::get<4>(_spatialStorageArea[b.second]);
                });

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromType = std::get<5>(area);
        if (areaId == entryId) {
          continue;
        }
        areas++;

        bool doesIntersect = false;
        std::string areaIRI =
            _writer->generateIRI(areaNamespace(areaFromType), areaObjId);

        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          intersectsSkippedByDAG++;
          doesIntersect = true;
        } else {
          intersectsChecks++;
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto start = std::chrono::steady_clock::now();
#endif

          doesIntersect = areaIntersectsArea(entry, area);
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto end = std::chrono::steady_clock::now();
          if (_config.writeGeometricRelationStatistics) {
            _statistics.write(statisticLine(
                __func__, "Area", "doesIntersect", areaId, "area", entryId,
                "area", std::chrono::nanoseconds(end - start), doesIntersect));
          }
#endif
          if (!doesIntersect) {
            continue;
          }
          intersectsOk++;

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
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

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
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
              << "contains: " << containsChecks << " yes: " << containsOk
              << std::endl;
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
      osm2rdf::geometry::Box nodeEnvelope;
      boost::geometry::envelope(std::get<1>(node), nodeEnvelope);
      const auto& nodeId = std::get<0>(node);
      std::string nodeIRI = _writer->generateIRI(
          osm2rdf::ttl::constants::NAMESPACE__OSM_NODE, nodeId);

      // Set containing all areas we are inside of
      std::set<osm2rdf::util::DirectedGraph<osm2rdf::osm::Area::id_t>::entry_t>
          skip;
      std::vector<SpatialAreaRefValue> queryResult;
      _spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                          std::back_inserter(queryResult));
      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
                [this](const auto& a, const auto& b) {
                  return std::get<4>(_spatialStorageArea[a.second]) <
                         std::get<4>(_spatialStorageArea[b.second]);
                });

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromType = std::get<5>(area);
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

        std::string areaIRI =
            _writer->generateIRI(areaNamespace(areaFromType), areaObjId);
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
    size_t containsSkippedByDAG = 0;
    size_t skippedInDAG = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared( \
    std::cout, std::cerr,\
    osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, nodeData, \
    osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, \
    osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA, \
    progressBar, entryCount, ia) reduction(+:areas,intersectsSkippedByDAG, containsSkippedByDAG, skippedInDAG, \
      intersectsByNodeInfo, intersectsChecks, intersectsOk, containsChecks, containsOk)    \
    default(none) schedule(dynamic)

    for (size_t i = 0; i < _numWays; i++) {
      SpatialWayValue way;

#pragma omp critical(loadEntry)
      ia >> way;

      const auto& wayEnvelopes = std::get<4>(way);
      const auto& wayId = std::get<1>(way);
      const auto& wayNodeIds = std::get<3>(way);

      // Check if our "area" has successors in the DAG, if yes we are part of
      // the DAG and don't need to calculate any relation again.

      const auto& successors = _directedAreaGraph.findSuccessorsFast(wayId * 2);

      if (!successors.empty()) {
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
      std::vector<SpatialAreaRefValue> queryResult;

      for (const auto& wayEnvelope : wayEnvelopes) {
        _spatialIndex.query(boost::geometry::index::intersects(wayEnvelope),
                            std::back_inserter(queryResult));
      }

      if (wayEnvelopes.size() > 1) {
        // remove duplicates, may occur since we used multiple envelope queries
        // to build the result!
        std::sort(
            queryResult.begin(), queryResult.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        auto last = std::unique(
            queryResult.begin(), queryResult.end(),
            [](const auto& a, const auto& b) { return a.second == b.second; });

        // duplicates were swapped to  the end of vector, beginning at last
        queryResult.erase(last, queryResult.end());
      }

      // small -> big
      std::sort(queryResult.begin(), queryResult.end(),
                [this](const auto& a, const auto& b) {
                  return std::get<4>(_spatialStorageArea[a.second]) <
                         std::get<4>(_spatialStorageArea[b.second]);
                });

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromType = std::get<5>(area);

        if (areaFromType == 1 && areaObjId == wayId) {
          continue;
        }

        areas++;

        bool doesIntersect = false;
        bool borderConted = false;

        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          intersectsSkippedByDAG++;
          doesIntersect = true;
        } else if (areaFromType == 0 && borderContained(wayId, areaObjId)) {
          doesIntersect = true;
          borderConted = true;

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNamespace(areaFromType), areaObjId);

          _writer->writeTriple(
              areaIRI,
              osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,
              wayIRI);
        } else if (skipNodeContained.find(areaId) != skipNodeContained.end()) {
          intersectsByNodeInfo++;
          doesIntersect = true;

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNamespace(areaFromType), areaObjId);

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

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNamespace(areaFromType), areaObjId);

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
          auto start = std::chrono::steady_clock::now();
#endif
          bool isCoveredBy = borderConted || wayInArea(way, area);
#ifdef ENABLE_GEOMETRY_STATISTIC
          auto end = std::chrono::steady_clock::now();
#endif

#ifdef ENABLE_GEOMETRY_STATISTIC
          if (_config.writeGeometricRelationStatistics) {
            printWayAreaStats(
                way, area,
                std::chrono::nanoseconds(end - start).count() / 1000);
            _statistics.write(statisticLine(
                __func__, "Way", "isCoveredBy2", areaId, "area", wayId, "way",
                std::chrono::nanoseconds(end - start), isCoveredBy));
          }
#endif
          if (!isCoveredBy) {
            continue;
          }
          containsOk++;

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipContains.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNamespace(areaFromType), areaObjId);

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
              << "contains: " << containsChecks << " yes: " << containsOk
              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " " << skippedInDAG
              << " ways are areas in the DAG -> skipped calculations for them"
              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << (static_cast<double>(areas) / (_numWays - skippedInDAG))
              << " areas checked per geometry on average" << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::nodeInArea(
    const SpatialNodeValue& a, const SpatialAreaValue& b) const {
  const auto& geomA = std::get<1>(a);
  int32_t ndBoxId = getBoxId(geomA);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& areaBoxIds = std::get<8>(b);

  const auto boxIdRes = nodeInAreaBoxIds(areaBoxIds, ndBoxId);

  if (boxIdRes == 2) return true;
  if (boxIdRes == 0) return false;

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    return boost::geometry::covered_by(geomA, geomB);
  }

  if (_config.approximateSpatialRels) {
    return boost::geometry::covered_by(geomA, outerGeomB);
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
bool osm2rdf::osm::GeometryHandler<W>::areaIntersectsArea(
    const SpatialAreaValue& a, const SpatialAreaValue& b) const {
  const auto& envelopesA = std::get<0>(a);
  const auto& envelopesB = std::get<0>(b);
  const auto& geomA = std::get<2>(a);
  const auto& geomB = std::get<2>(b);
  const auto& innerGeomA = std::get<6>(a);
  const auto& outerGeomA = std::get<7>(a);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& boxIdsA = std::get<8>(a);
  const auto& boxIdsB = std::get<8>(b);

  bool intersects = false;
  for (size_t i = 1; i < envelopesB.size(); i++) {
    if (boost::geometry::intersects(envelopesA[0], envelopesB[i]))
      intersects = true;
  }
  if (!intersects) return false;

  const auto boxIdRes = areaIntersectsAreaBoxIds(boxIdsA, boxIdsB);

  if (boxIdRes == 2) return true;
  if (boxIdRes == 0) return false;

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    return boost::geometry::intersects(geomA, geomB);
  }

  if (boost::geometry::intersects(innerGeomA, innerGeomB)) {
    // if simplified inner intersect, we definitely intersect
    return true;
  } else if (!boost::geometry::intersects(outerGeomA, outerGeomB)) {
    // if NOT intersecting simplified outer, we are definitely NOT
    // intersecting
    return false;
  } else if (boost::geometry::intersects(geomA, geomB)) {
    return true;
  }

  return false;
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::wayIntersectsArea(
    const SpatialWayValue& a, const SpatialAreaValue& b) const {
  const auto& geomA = std::get<2>(a);
  const auto& wayBoxIds = std::get<5>(a);
  const auto& wayEnvelope = std::get<0>(a);

  const auto& geomB = std::get<2>(b);
  const auto& envelopesB = std::get<0>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& areaBoxIds = std::get<8>(b);

  bool intersects = false;
  for (size_t i = 1; i < envelopesB.size(); i++) {
    if (boost::geometry::intersects(wayEnvelope, envelopesB[i])) {
      intersects = true;
      break;
    }
  }
  if (!intersects) return false;

  const auto boxIdRes = wayIntersectsAreaBoxIds(wayBoxIds, areaBoxIds);

  if (boxIdRes == 2) {
    return true;
  }
  if (boxIdRes == 0) {
    return false;
  }

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    if (!boost::geometry::intersects(geomA, envelopesB[0]))
      // if does not intersect with envelope, we definitely dont intersect
      return false;

    return boost::geometry::intersects(geomA, geomB);
  }

  if (_config.approximateSpatialRels) {
    return boost::geometry::intersects(geomA, outerGeomB);
  }

  intersects = false;
  for (size_t i = 1; i < envelopesB.size(); i++) {
    if (boost::geometry::intersects(geomA, envelopesB[i])) intersects = true;
  }

  if (!intersects) {
    // if does not intersect with envelope, we definitely dont intersect
    return false;
  } else if (boost::geometry::intersects(geomA, innerGeomB)) {
    // if intersects simplified inner, we definitely intersect
    return true;
  } else if (!boost::geometry::intersects(geomA, outerGeomB)) {
    // if NOT intersecting simplified outer, we are definitely NOT
    // intersecting
    return false;
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
  const auto& wayBoxIds = std::get<5>(a);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& envelopesB = std::get<0>(b);
  const auto& areaBoxIds = std::get<8>(b);

  bool covered = false;
  for (size_t i = 1; i < envelopesB.size(); i++) {
    if (boost::geometry::covered_by(envelopeA, envelopesB[i])) {
      covered = true;
      break;
    }
  }
  if (!covered) return false;

  const auto boxIdRes = wayInAreaBoxIds(wayBoxIds, areaBoxIds);

  if (boxIdRes == 2) {
    return true;
  }
  if (boxIdRes == 0) {
    return false;
  }

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    return boost::geometry::covered_by(geomA, geomB);
  }

  if (_config.approximateSpatialRels) {
    return boost::geometry::covered_by(geomA, outerGeomB);
  }

  boost::geometry::model::ring<osm2rdf::geometry::Location> ring;
  ring.push_back(envelopeA.min_corner());
  ring.push_back(
      {envelopeA.min_corner().get<0>(), envelopeA.max_corner().get<1>()});
  ring.push_back(envelopeA.max_corner());
  ring.push_back(
      {envelopeA.max_corner().get<0>(), envelopeA.min_corner().get<1>()});
  ring.push_back(envelopeA.min_corner());
  osm2rdf::geometry::Polygon bboxPoly;
  bboxPoly.outer() = ring;

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
  const auto& areaA = std::get<4>(a);
  const auto& innerGeomA = std::get<6>(a);
  const auto& outerGeomA = std::get<7>(a);
  const auto& boxIdsA = std::get<8>(a);
  const auto& envelopesA = std::get<0>(a);

  const auto& geomB = std::get<2>(b);
  const auto& areaB = std::get<4>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& boxIdsB = std::get<8>(b);
  const auto& envelopesB = std::get<0>(b);

  if (areaA > areaB) return false;
  if (!boost::geometry::covered_by(envelopesA[0], envelopesB[0])) return false;

  const auto boxIdRes = areaInAreaBoxIds(boxIdsA, boxIdsB);

  if (boxIdRes == 2) return true;
  if (boxIdRes == 0) return false;

  if (_config.dontUseInnerOuterGeoms || boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB) ||
      boost::geometry::is_empty(outerGeomA) ||
      boost::geometry::is_empty(innerGeomA)) {
    return boost::geometry::covered_by(geomA, geomB);
  }

  if (boost::geometry::covered_by(outerGeomA, innerGeomB)) {
    // if simplified outer is covered by simplified inner, we are
    // definitely contained
    return true;
  }
  // } else if (!boost::geometry::covered_by(innerGeomA, outerGeomB)) {
  // // if simplified inner is not covered by simplified outer, we are
  // // definitely not contained
  // // assert(!boost::geometry::covered_by(geomA, geomB));
  // return false;
  // } else if (boost::geometry::covered_by(entryGeom, areaInnerGeom)) {
  // // if covered by simplified inner, we are definitely contained
  // isCoveredBy = true;
  // } else if (!boost::geometry::covered_by(entryGeom, areaOuterGeom))
  // {
  // // if NOT covered by simplified out, we are definitely not
  // contained isCoveredBy = false;

  if (boost::geometry::covered_by(geomA, geomB)) {
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
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::printWayAreaStats(
    const SpatialWayValue& way, const SpatialAreaValue& area, double usec) {
  const auto& areaGeom = std::get<2>(area);
  const auto& areaGeomInner = std::get<6>(area);
  const auto& areaGeomOuter = std::get<7>(area);

  const auto& wayGeom = std::get<2>(way);

  size_t numWayPoints = wayGeom.size();
  size_t numAreaPoints = 0;

  const auto& areaOSMId = std::get<3>(area);
  const auto& wayOSMId = std::get<1>(way);

  // take total number of points of all polygons in multipolygon, also counting
  // inners
  for (const auto& p : areaGeom) {
    numAreaPoints += p.outer().size();
    for (const auto& inner : p.inners()) {
      numAreaPoints += inner.size();
    }
  }

  std::stringstream ss;
  std::stringstream m;
  std::string area_prefix = std::get<5>(area) ? "osmway:" : "osmrel:";
  m << "osm2rdf:check_contains_" << wayOSMId << "_" << areaOSMId;
  ss << "osmway:" << wayOSMId << " osm2rdf:num_points "
     << "\"" << numWayPoints << "\"^^xsd:int . \n"
     << area_prefix << areaOSMId << " osm2rdf:num_points "
     << "\"" << numAreaPoints << "\"^^xsd:int . \n"
     << "osmway:" << wayOSMId << " osm2rdf:check_contains_to " << m.str()
     << " . \n"
     << m.str() << " osm2rdf:check_contains_from " << area_prefix << areaOSMId
     << " . \n"
     << m.str() << " osm2rdf:check_contains_usecs "
     << "\"" << usec << "\"^^xsd:int . \n"
     << m.str() << " osm2rdf:check_contains_usecs_div_10 "
     << "\"" << (int)(usec / 10) << "\"^^xsd:int . \n";
  _containsStatistics.write(ss.str());
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::addDummyRegion(
    osm2rdf::geometry::Area dummy, double dummyArea) {
  auto innerGeom = osm2rdf::geometry::Area();
  auto outerGeom = osm2rdf::geometry::Area();

  FactHandler factHandler(_config, _writer);

  if (!_config.dontUseInnerOuterGeoms) {
    innerGeom = simplifiedArea(dummy, true);
    outerGeom = simplifiedArea(dummy, false);
  }

#pragma omp critical(addDummyGeom)
  {
    _dummyAreaCount++;

    size_t dummyId = std::numeric_limits<id_t>::max() - _dummyAreaCount;
    size_t dummyObjId = _dummyAreaCount;

    osm2rdf::geometry::Box box;
    boost::geometry::envelope(dummy, box);

    _spatialStorageArea.push_back(SpatialAreaValue(
        {box, box}, dummyId, dummy, dummyObjId, dummyArea, 2, innerGeom,
        outerGeom, pack(getBoxIds(dummy, box, innerGeom, outerGeom))));

    std::string s = _writer->generateIRI(
        osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_PARTITION, dummyObjId);

    factHandler.writeBoostGeometry(
        s, osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY, dummy);
  }
}

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::BoxIdList osm2rdf::osm::GeometryHandler<W>::getBoxIds(
    const osm2rdf::geometry::Way& way,
    const osm2rdf::geometry::Box& envelope) const {
  int32_t startX = floor((envelope.min_corner().get<0>() + 180.0) / GRID_W);
  int32_t startY = floor((envelope.min_corner().get<1>() + 90.0) / GRID_H);

  int32_t endX = ceil((envelope.max_corner().get<0>() + 180.0) / GRID_W);
  int32_t endY = ceil((envelope.max_corner().get<1>() + 90.0) / GRID_H);

  osm2rdf::osm::BoxIdList boxIds;

  for (int32_t y = startY; y < endY; y++) {
    for (int32_t x = startX; x < endX; x++) {
      osm2rdf::geometry::Box box;
      box.min_corner().set<0>(x * GRID_W - 180.0);
      box.min_corner().set<1>(y * GRID_H - 90.0);
      box.max_corner().set<0>((x + 1) * GRID_W - 180.0);
      box.max_corner().set<1>((y + 1) * GRID_H - 90.0);

      if (boost::geometry::intersects(way, box)) {
        int32_t newId = y * NUM_GRID_CELLS + x + 1;
        if (boxIds.size() && boxIds.back().second < 254 &&
            boxIds.back().first + boxIds.back().second == newId - 1)
          boxIds.back().second++;
        else
          boxIds.push_back({newId, 0});
      }
    }
  }

  return boxIds;
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::GeometryHandler<W>::getBoxIds(
    const osm2rdf::geometry::Area& area,

    const osm2rdf::geometry::Area& inner, const osm2rdf::geometry::Area& outer,
    int xFrom, int xTo, int yFrom, int yTo, int xWidth, int yHeight,
    osm2rdf::osm::BoxIdList* ret) const {
  for (int32_t y = yFrom; y < yTo; y += yHeight) {
    for (int32_t x = xFrom; x < xTo; x += xWidth) {
      int localXWidth = std::min(xTo - x, xWidth);
      int localYHeight = std::min(yTo - y, yHeight);

      osm2rdf::geometry::Box box;
      box.min_corner().set<0>(x * GRID_W - 180.0);
      box.min_corner().set<1>(y * GRID_H - 90.0);
      box.max_corner().set<0>((x + localXWidth + 1) * GRID_W - 180.0);
      box.max_corner().set<1>((y + localYHeight + 1) * GRID_H - 90.0);

      boost::geometry::model::ring<osm2rdf::geometry::Location> ring;
      ring.push_back(box.min_corner());
      ring.push_back({box.min_corner().get<0>(), box.max_corner().get<1>()});
      ring.push_back(box.max_corner());
      ring.push_back({box.max_corner().get<0>(), box.min_corner().get<1>()});
      ring.push_back(box.min_corner());
      osm2rdf::geometry::Polygon bboxPoly;
      bboxPoly.outer() = ring;

      bool outerIsects = boost::geometry::is_empty(outer) ||
                         boost::geometry::intersects(outer, box);
      bool innerIsects = outerIsects && !boost::geometry::is_empty(inner) &&
                         boost::geometry::intersects(inner, box);

      if (outerIsects &&
          (innerIsects || boost::geometry::intersects(area, box))) {
        bool outerConted = boost::geometry::is_empty(outer) ||
                           boost::geometry::covered_by(bboxPoly, outer);
        bool innerConted = outerConted && !boost::geometry::is_empty(inner) &&
                           boost::geometry::covered_by(bboxPoly, inner);

        if (outerConted &&
            (innerConted || boost::geometry::covered_by(bboxPoly, area))) {
          // we can insert all at once
          for (int32_t ly = y; ly < y + localYHeight; ly++) {
            int a = 1;
            int start = ly * NUM_GRID_CELLS + x;
            while (a <= (localXWidth + ((255 + 1) - 1)) / (255 + 1)) {
              int r = std::min(
                  255, start + localXWidth - (start + (a - 1) * (255 + 1)) - 1);
              ret->push_back({start + (a - 1) * (255 + 1) + 1, r});
              a++;
            }
          }
        } else {
          if (localXWidth == 1 && localYHeight == 1) {
            int32_t newId = -(y * NUM_GRID_CELLS + x + 1);
            if (ret->size() && ret->back().second < 254 &&
                ret->back().first - ret->back().second == newId + 1)
              ret->back().second++;
            else
              ret->push_back({newId, 0});
          } else {
            // we need to check in detail on a smaller level!
            // recurse down...
            int newXWidth = (localXWidth + 1) / 2;
            int newYHeight = (localYHeight + 1) / 2;

            getBoxIds(area, inner, outer, x, x + localXWidth, y,
                      y + localYHeight, newXWidth, newYHeight, ret);
          }
        }
      }
    }
  }
}

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::BoxIdList osm2rdf::osm::GeometryHandler<W>::getBoxIds(
    const osm2rdf::geometry::Area& area, const osm2rdf::geometry::Box& envelope,
    const osm2rdf::geometry::Area& inner,
    const osm2rdf::geometry::Area& outer) const {
  size_t totNumPoints = 0;
  for (const auto& p : area) totNumPoints += p.outer().size();

  int32_t startX = floor((envelope.min_corner().get<0>() + 180.0) / GRID_W);
  int32_t startY = floor((envelope.min_corner().get<1>() + 90.0) / GRID_H);

  int32_t endX = ceil((envelope.max_corner().get<0>() + 180.0) / GRID_W);
  int32_t endY = ceil((envelope.max_corner().get<1>() + 90.0) / GRID_H);

  osm2rdf::osm::BoxIdList boxIds;

  getBoxIds(area, inner, outer, startX, endX, startY, endY,
            (endX - startX + 3) / 4, (endY - startY + 3) / 4, &boxIds);
  std::sort(boxIds.begin(), boxIds.end(), BoxIdCmp());

  return boxIds;
}

// ____________________________________________________________________________
template <typename W>
int32_t osm2rdf::osm::GeometryHandler<W>::getBoxId(
    const osm2rdf::geometry::Location& p) const {
  int32_t x = floor((p.get<0>() + 180.0) / GRID_W);
  int32_t y = floor((p.get<1>() + 90.0) / GRID_H);

  return y * NUM_GRID_CELLS + x + 1;
}

// ____________________________________________________________________________
template <typename W>
std::string osm2rdf::osm::GeometryHandler<W>::areaNamespace(
    uint8_t type) const {
  switch (type) {
    case 0:
      return osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION;
    case 1:
      return osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
    case 2:
      return osm2rdf::ttl::constants::NAMESPACE__OSM2RDF_PARTITION;
    default:
      return osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
  }
}

// ____________________________________________________________________________
template <typename W>
uint8_t osm2rdf::osm::GeometryHandler<W>::polyIntersectPolyBoxIds(
    const osm2rdf::osm::BoxIdList& wayBoxIds,
    const osm2rdf::osm::BoxIdList& areaBoxIds, bool contains) const {
  if (areaBoxIds.size() == 0 || wayBoxIds.size() == 0) return 1;

  // shortcuts
  if (abs(wayBoxIds[1].first) >
      abs(areaBoxIds.back().first) + areaBoxIds.back().second)
    return 0;
  if (abs(wayBoxIds.back().first) + wayBoxIds.back().second <
      areaBoxIds[1].first)
    return 0;

  uint8_t res = 0;

  size_t numEq = 0;

  // 0: no res
  // 1: possibly in
  // 2: in

  size_t i = 1;
  int32_t ii = 0;
  size_t j = 1;
  int32_t jj = 0;

  size_t numTot = wayBoxIds[0].first;

  while (i < wayBoxIds.size() && j < areaBoxIds.size()) {
    if (abs(wayBoxIds[i].first) + ii == abs(areaBoxIds[j].first) + jj) {
      if (areaBoxIds[j].first > 0 && res == 0) res = 2;
      if (areaBoxIds[j].first > 0 && !contains) return 2;
      if (areaBoxIds[j].first < 0) res = 1;

      numEq++;

      if (++ii > wayBoxIds[i].second) {
        i++;
        ii = 0;
      }
      if (++jj > areaBoxIds[j].second) {
        j++;
        jj = 0;
      }
    } else if (abs(wayBoxIds[i].first) + ii < abs(areaBoxIds[j].first) + jj) {
      if (contains) return 0;

      if (abs(wayBoxIds[i].first) + wayBoxIds[i].second <
          abs(areaBoxIds[j].first) + jj) {
        // entire block smaller, jump it
        ii = 0;
        i++;
      } else {
        if (++ii > wayBoxIds[i].second) {
          i++;
          ii = 0;
        }
      }
    } else {
      size_t gallop = 1;
      do {
        auto end = areaBoxIds.end();
        if (j + gallop < areaBoxIds.size())
          end = areaBoxIds.begin() + j + gallop;

        if (end == areaBoxIds.end() ||
            abs(end->first) >= abs(wayBoxIds[i].first) + ii) {
          jj = 0;
          j = std::lower_bound(areaBoxIds.begin() + j + gallop / 2, end,
                               wayBoxIds[i].first + ii, BoxIdCmp()) -
              areaBoxIds.begin();
          if (j > 0 &&
              abs(areaBoxIds[j - 1].first) < abs(wayBoxIds[i].first) + ii &&
              abs(areaBoxIds[j - 1].first) + areaBoxIds[j - 1].second >=
                  abs(wayBoxIds[i].first) + ii) {
            j--;
            jj = (abs(wayBoxIds[i].first) + ii) - abs(areaBoxIds[j].first);
          }
          break;
        }

        gallop *= 2;

      } while (true);
    }
  }

  if (contains && numEq != numTot) return 0;

  return res;
}

// ____________________________________________________________________________
template <typename W>
uint8_t osm2rdf::osm::GeometryHandler<W>::nodeInAreaBoxIds(
    const osm2rdf::osm::BoxIdList& areaBoxIds, int32_t ndBoxId) const {
  return wayIntersectsAreaBoxIds({{1, 0}, {ndBoxId, 0}}, areaBoxIds);
}

// ____________________________________________________________________________
template <typename W>
uint8_t osm2rdf::osm::GeometryHandler<W>::wayIntersectsAreaBoxIds(
    const osm2rdf::osm::BoxIdList& way,
    const osm2rdf::osm::BoxIdList& area) const {
  return polyIntersectPolyBoxIds(way, area, false);
}

// ____________________________________________________________________________
template <typename W>
uint8_t osm2rdf::osm::GeometryHandler<W>::wayInAreaBoxIds(
    const osm2rdf::osm::BoxIdList& way,
    const osm2rdf::osm::BoxIdList& area) const {
  return polyIntersectPolyBoxIds(way, area, true);
}
// ____________________________________________________________________________
template <typename W>
uint8_t osm2rdf::osm::GeometryHandler<W>::areaInAreaBoxIds(
    const osm2rdf::osm::BoxIdList& a, const osm2rdf::osm::BoxIdList& b) const {
  return polyIntersectPolyBoxIds(a, b, true);
}
// ____________________________________________________________________________
template <typename W>
uint8_t osm2rdf::osm::GeometryHandler<W>::areaIntersectsAreaBoxIds(
    const osm2rdf::osm::BoxIdList& a, const osm2rdf::osm::BoxIdList& b) const {
  return polyIntersectPolyBoxIds(a, b, false);
}

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::BoxIdList osm2rdf::osm::GeometryHandler<W>::pack(
    const osm2rdf::osm::BoxIdList& ids) const {
  if (ids.size() == 0) return ids;
  // assume the list is sorted!

  osm2rdf::osm::BoxIdList ret;
  // dummy value, will later hold number of entries
  ret.push_back({ids.front().second + 1, 0});
  ret.push_back(ids.front());

  for (size_t i = 1; i < ids.size(); i++) {
    ret[0].first += ids[i].second + 1;
    if (ret.back().second < 254 - ids[i].second && ids[i].first > 0 &&
        ret.back().first > 0 &&
        ret.back().first + ret.back().second == ids[i].first - 1)
      ret.back().second += 1 + ids[i].second;
    else if (ret.back().second < 254 - ids[i].second && ids[i].first < 0 &&
             ret.back().first < 0 &&
             ret.back().first - ret.back().second == ids[i].first + 1)
      ret.back().second += 1 + ids[i].second;
    else
      ret.push_back(ids[i]);
  }

  return ret;
}

// ____________________________________________________________________________
template <typename W>
bool osm2rdf::osm::GeometryHandler<W>::borderContained(
    osm2rdf::osm::Way::id_t wayId, osm2rdf::osm::Area::id_t areaObjId) const {
  const auto& relations = _areaBorderWaysIndex.find(wayId);
  if (relations != _areaBorderWaysIndex.end()) {
    auto a =
        std::lower_bound(relations->second.begin(), relations->second.end(), areaObjId);

    if (a != relations->second.end() && (*a) == areaObjId) {
      return true;
    }
  }
  return false;
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER>;

#pragma clang diagnostic pop
