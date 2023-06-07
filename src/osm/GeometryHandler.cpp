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
#include "osm2rdf/ttl/Constants.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/DirectedAcyclicGraph.h"
#include "osm2rdf/util/DirectedGraph.h"
#include "osm2rdf/util/ProgressBar.h"
#include "osm2rdf/util/Time.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-loop-convert"

using osm2rdf::osm::Area;
using osm2rdf::osm::BoxIdList;
using osm2rdf::osm::GeometryHandler;
using osm2rdf::osm::Node;
using osm2rdf::osm::Relation;
using osm2rdf::osm::SpatialAreaRefValue;
using osm2rdf::osm::Way;
using osm2rdf::osm::constants::BASE_SIMPLIFICATION_FACTOR;
using osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_AREA;
using osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA;
using osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA;
using osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA;
using osm2rdf::ttl::constants::NAMESPACE__OSM_NODE;
using osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
using osm2rdf::util::currentTimeFormatted;
using osm2rdf::util::DirectedGraph;

// ____________________________________________________________________________
template <typename W>
GeometryHandler<W>::GeometryHandler(const osm2rdf::config::Config& config,
                                    osm2rdf::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer),
      _tmpPathUnnamedAreas(config.getTempPath("spatial", "areas_unnamed")),
      _fsUnnamedAreas(_tmpPathUnnamedAreas, std::ios::in | std::ios::out |
                                                std::ios::trunc |
                                                std::ios::binary),
      _oaUnnamedAreas(_fsUnnamedAreas),
      _tmpPathNodes(config.getTempPath("spatial", "nodes")),
      _fsNodes(_tmpPathNodes, std::ios::in | std::ios::out | std::ios::trunc |
                                  std::ios::binary),
      _oaNodes(_fsNodes),
      _tmpPathWays(config.getTempPath("spatial", "ways")),
      _fsWays(_tmpPathWays, std::ios::in | std::ios::out | std::ios::trunc |
                                std::ios::binary),
      _oaWays(_fsWays) {
  // immediately unlink all opened files to ensure their removal at exit / crash
  // note that if one of the initializations above fails after a file was
  // opened, it will not be deleted
  unlink(_tmpPathUnnamedAreas.c_str());
  unlink(_tmpPathNodes.c_str());
  unlink(_tmpPathWays.c_str());

  _fsUnnamedAreas << std::fixed << std::setprecision(_config.wktPrecision);
  _fsWays << std::fixed << std::setprecision(_config.wktPrecision);
  _fsNodes << std::fixed << std::setprecision(_config.wktPrecision);
}

// ___________________________________________________________________________
template <typename W>
GeometryHandler<W>::~GeometryHandler() = default;

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::relation(const Relation& rel) {
  const auto& tags = rel.tags();
  auto typeTag = tags.find("type");
  if (typeTag != tags.end() &&
      (typeTag->second == "boundary" || typeTag->second == "multipolygon")) {
    for (const auto& member : rel.members()) {
      if (member.type() == RelationMemberType::WAY &&
          (member.role() == "outer" || member.role() == "inner")) {
#pragma omp critical
        {
          _areaBorderWaysIndex[member.id()].push_back(
              {rel.id(), member.role() == "inner"});
          std::sort(_areaBorderWaysIndex[member.id()].begin(),
                    _areaBorderWaysIndex[member.id()].end(), MemberRelCmp());
        }
      }
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::area(const Area& area) {
  // inner simplified geom, empty by default
  osm2rdf::geometry::Area innerGeom;

  // outer simplified geom, empty by default
  osm2rdf::geometry::Area outerGeom;

  // the convex hull of the geometry
  osm2rdf::geometry::Polygon convexHull;

  const auto& geom = simplifyGeometry(area.geom());

  size_t totPoints = 0;
  const size_t MIN_CUTOUT_POINTS = 10000;

  for (const auto& poly : geom) {
    totPoints += poly.outer().size();
  }

  if ((_config.geometryOptimizations &
       osm2rdf::config::GeometryRelationOptimization::APPROXIMATE_POLYGONS) &&
      (area.hasName() || !area.fromWay())) {
    innerGeom = simplifiedArea(geom, true);
    outerGeom = simplifiedArea(geom, false);
  }

  if (!geom.empty()) {
    convexHull = area.convexHull();
    if (convexHull.outer().size() >= geom.front().outer().size() / 2) {
      // if we haven't gained anything significant in terms of number of anchor
      // points, discard
      convexHull = osm2rdf::geometry::Polygon();
    }
  }

  // add the bounding boxes of the entire (multi-) polygon, and of all its
  // individual components
  std::vector<osm2rdf::geometry::Box> envelopes;
  envelopes.push_back(area.envelope());
  if (area.geom().size() > 1) {
    for (const auto& polygon : area.geom()) {
      osm2rdf::geometry::Box box;
      boost::geometry::envelope(polygon, box);
      envelopes.push_back(box);
    }
  } else {
    // if only one element was contained, don't recalculate the entire bounding
    // box
    envelopes.push_back(area.envelope());
  }

  std::unordered_map<int32_t, osm2rdf::geometry::Area> cutouts;

  const auto& boxIds =
      pack(getBoxIds(area.geom(), envelopes, innerGeom, outerGeom,
                     totPoints > MIN_CUTOUT_POINTS ? &cutouts : 0));

#pragma omp critical(areaDataInsert)
  {
    if (!_config.splitNamedAndUnnamedAreas || area.hasName()) {
      _spatialStorageArea.push_back(
          {envelopes, area.id(), geom, area.objId(), area.geomArea(),
           area.fromWay() ? AreaFromType::WAY : AreaFromType::RELATION,
           innerGeom, outerGeom, boxIds, cutouts, convexHull,
           area.orientedBoundingBox()});
    } else if (!area.fromWay()) {
      // Areas from ways are handled in GeometryHandler<W>::way
      _oaUnnamedAreas << SpatialAreaValue(
          envelopes, area.id(), geom, area.objId(), area.geomArea(),
          area.fromWay() ? AreaFromType::WAY : AreaFromType::RELATION,
          innerGeom, outerGeom, boxIds, cutouts, convexHull,
          area.orientedBoundingBox());
      _numUnnamedAreas++;
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::node(const Node& node) {
#pragma omp critical(nodeDataInsert)
  {
    _oaNodes << SpatialNodeValue(node.id(), node.geom());
    _numNodes++;
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::way(const Way& way) {
  WayNodeList nodeIds;
  nodeIds.reserve(way.nodes().size());

  for (const auto& nodeRef : way.nodes()) {
    nodeIds.push_back(nodeRef.id());
  }
  const auto& geom = simplifyGeometry(way.geom());

  std::vector<osm2rdf::geometry::Box> boxes;

  const size_t CHUNKSIZE = 25;

  // for large ways, sample individual smaller bounding boxes to allow for
  // fine-grained R-tree requests later on
  if (geom.size() >= 2 * CHUNKSIZE) {
    size_t curSize = 0;
    while (curSize < geom.size()) {
      osm2rdf::geometry::Box box;

      osm2rdf::geometry::Way tmp(
          geom.begin() + curSize,
          geom.begin() + std::min(curSize + CHUNKSIZE, geom.size()));

      boost::geometry::envelope(tmp, box);
      boxes.push_back(box);
      curSize += CHUNKSIZE;
    }
  } else {
    boxes.push_back(way.envelope());
  }

  const auto& boxIds = pack(getBoxIds(geom, way.envelope()));

#pragma omp critical(wayDataInsert)
  {
    _oaWays << SpatialWayValue(way.envelope(), way.id(), geom, nodeIds, boxes,
                               boxIds, way.convexHull(),
                               way.orientedBoundingBox());

    _spatialStorageWay.push_back(
        {way.envelope(), way.id(), geom, nodeIds, boxes,
                               boxIds, way.convexHull(),
                               way.orientedBoundingBox()});

    _numWays++;
  }
}

// ____________________________________________________________________________
template <typename W>
template <typename G>
G GeometryHandler<W>::simplifyGeometry(const G& geom) const {
  G simplifiedGeom;

  if (_config.simplifyGeometries == 0) {
    // simple case, just remove colinear points
    boost::geometry::simplify(geom, simplifiedGeom, 0);
    return simplifiedGeom;
  }

  auto perimeterOrLength =
      std::max(boost::geometry::perimeter(geom), boost::geometry::length(geom));
  do {
    boost::geometry::simplify(geom, simplifiedGeom,
                              BASE_SIMPLIFICATION_FACTOR * perimeterOrLength *
                                  _config.simplifyGeometries);
    perimeterOrLength /= 2;
  } while ((boost::geometry::is_empty(simplifiedGeom) ||
            !boost::geometry::is_valid(simplifiedGeom)) &&
           perimeterOrLength >= BASE_SIMPLIFICATION_FACTOR);
  if (!boost::geometry::is_valid(simplifiedGeom)) {
    return geom;
  }
  if (boost::geometry::is_empty(simplifiedGeom)) {
    return geom;
  }
  return simplifiedGeom;
}

// ____________________________________________________________________________
template <typename W>
template <osm2rdf::osm::InnerOuterDouglasPeuckerMode MODE>
bool GeometryHandler<W>::innerOuterDouglasPeucker(
    const boost::geometry::model::ring<osm2rdf::geometry::Location>&
        inputPoints,
    boost::geometry::model::ring<osm2rdf::geometry::Location>& outputPoints,
    size_t l, size_t r, double eps) const {
  // this is basically a verbatim translation from Hannah's qlever map UI code

  assert(r >= l);
  assert(!inputPoints.empty());
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
  if (MODE == InnerOuterDouglasPeuckerMode::INNER) {
    simplify = (max_dist_left == 0 && max_dist_right <= eps);
    m = max_dist_left > 0 ? m_left : m_right;
  }

  // OUTER Douglas-Peucker: Simplify iff there is no point to the *right*
  // *and* the leftmost point has distance <= eps. Otherwise m is the rightmost
  // point or if there is no such point the leftmost point.
  if (MODE == InnerOuterDouglasPeuckerMode::OUTER) {
    simplify = (max_dist_right == 0 && max_dist_left <= eps);
    m = max_dist_right > 0 ? m_right : m_left;
  }

  // Simplification case: If m is at most eps away from the line segment
  // connecting l and r, we can simplify the part of the polygon from l to r by
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
  bool a = innerOuterDouglasPeucker<MODE>(inputPoints, outputPoints, l, m, eps);
  bool b =
      innerOuterDouglasPeucker<MODE>(inputPoints, outputPoints, m + 1, r, eps);

  return a || b;
}

// ____________________________________________________________________________
template <typename W>
double GeometryHandler<W>::signedDistanceFromPointToLine(
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
osm2rdf::geometry::Area GeometryHandler<W>::simplifiedArea(
    const osm2rdf::geometry::Area& area, bool inner) const {
  // default value: empty area, means no inner geom
  osm2rdf::geometry::Area ret;

  // skip trivial / erroneous geoms
  if (!boost::geometry::is_valid(area)) {
    return ret;
  }

  if (boost::geometry::is_empty(area)) {
    return ret;
  }

  size_t numPointsOld = 0;
  size_t numPointsNew = 0;

  for (const auto& poly : area) {
    osm2rdf::geometry::Polygon simplified;
    numPointsOld += poly.outer().size();

    for (const auto& origInner : poly.inners()) {
      numPointsOld += origInner.size();
      if (origInner.size() < 4) {
        numPointsNew += origInner.size();
        simplified.inners().push_back(origInner);
        continue;
      }

      // inner polygons are given in counter-clockwise order

      double eps = sqrt(-boost::geometry::area(origInner) / 3.14) * 3.14 * 2 *
                   _config.simplifyGeometriesInnerOuter;

      // simplify the inner geometries with outer simplification, because
      // inner geometries are given counter-clockwise, it is not
      // necessary to swap the simplification mode
      boost::geometry::model::ring<osm2rdf::geometry::Location> retDP;
      size_t m = floor(origInner.size() / 2);
      if (inner) {
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::INNER>(
            origInner, retDP, 0, m, eps);
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::INNER>(
            origInner, retDP, m + 1, origInner.size() - 1, eps);
      } else {
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::OUTER>(
            origInner, retDP, 0, m, eps);
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::OUTER>(
            origInner, retDP, m + 1, origInner.size() - 1, eps);
      }
      retDP.push_back(retDP.front());  // ensure valid polygon
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
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::INNER>(
            poly.outer(), retDP, 0, m, eps);
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::INNER>(
            poly.outer(), retDP, m + 1, poly.outer().size() - 1, eps);
      } else {
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::OUTER>(
            poly.outer(), retDP, 0, m, eps);
        innerOuterDouglasPeucker<InnerOuterDouglasPeuckerMode::OUTER>(
            poly.outer(), retDP, m + 1, poly.outer().size() - 1, eps);
      }
      retDP.push_back(retDP.front());  // ensure valid polygon
      numPointsNew += retDP.size();
      simplified.outer() = retDP;
    }

    ret.push_back(simplified);
  }

  if (numPointsNew >= numPointsOld) {
    // gain too low, return empty poly to avoid extra space and double-checking
    // later on
    return osm2rdf::geometry::Area();
  }

  return ret;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::flushExternalStorage() {
  if (_fsNodes.is_open()) {
    _fsNodes.flush();
  }
  if (_fsUnnamedAreas.is_open()) {
    _fsUnnamedAreas.flush();
  }
  if (_fsWays.is_open()) {
    _fsWays.flush();
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::calculateRelations() {
  // Ensure functions can open external storage for reading.
  flushExternalStorage();

  prepareRTree();
  prepareDAG();
  dumpNamedAreaRelations();
  dumpUnnamedAreaRelations();
  const auto& nodeData = dumpNodeRelations();
  dumpWayRelations(nodeData);

  prepareWayRTree();
  dumpWayWayRelations();
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::prepareRTree() {
  std::cerr << std::endl;
  std::cerr << currentTimeFormatted() << " Sorting "
            << _spatialStorageArea.size() << " areas ... " << std::endl;
  std::sort(_spatialStorageArea.begin(), _spatialStorageArea.end(),
            [](const auto& a, const auto& b) {
              return std::get<4>(a) > std::get<4>(b);
            });
  std::cerr << currentTimeFormatted() << " ... done " << std::endl;

  std::cerr << currentTimeFormatted() << " Packing area r-tree with "
            << _spatialStorageArea.size() << " entries ... " << std::endl;

  std::vector<SpatialAreaRefValue> values;

  for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
    for (size_t j = 1; j < std::get<0>(_spatialStorageArea[i]).size(); j++) {
      values.emplace_back(std::get<0>(_spatialStorageArea[i])[j], i);
    }
  }

  _spatialIndex = SpatialIndex(values.begin(), values.end());

  std::cerr << currentTimeFormatted() << " ... done" << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::prepareWayRTree() {
  std::cerr << std::endl;

  std::cerr << currentTimeFormatted() << " Packing area r-tree with "
            << _spatialStorageWay.size() << " entries ... " << std::endl;

  std::vector<SpatialAreaRefValue> values;

  for (size_t i = 0; i < _spatialStorageWay.size(); i++) {
    for (size_t j = 0; j < std::get<4>(_spatialStorageWay[i]).size(); j++) {
      values.emplace_back(std::get<4>(_spatialStorageWay[i])[j], i);
    }
  }

  _spatialWayIndex = SpatialIndex(values.begin(), values.end());

  std::cerr << currentTimeFormatted() << " ... done" << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::prepareDAG() {
  // Store dag
  DirectedGraph<Area::id_t> tmpDirectedAreaGraph;
  {
    // Prepare id based lookup table for later usage...
    _spatialStorageAreaIndex.reserve(_spatialStorageArea.size());
    for (size_t i = 0; i < _spatialStorageArea.size(); ++i) {
      const auto& area = _spatialStorageArea[i];
      _spatialStorageAreaIndex[std::get<1>(area)] = i;
    }

    std::cerr << currentTimeFormatted() << " Generating non-reduced DAG from "
              << _spatialStorageArea.size() << " areas ... " << std::endl;

    osm2rdf::util::ProgressBar progressBar{_spatialStorageArea.size(), true};
    size_t entryCount = 0;

    GeomRelationStats stats;

    progressBar.update(entryCount);

#pragma omp parallel for shared(                                               \
        tmpDirectedAreaGraph, std::cout, std::cerr,                            \
            osm2rdf::ttl::constants::IRI__GEOSPARQL__HAS_GEOMETRY, entryCount, \
            progressBar) reduction(+ : stats) default(none) schedule(dynamic)

    for (size_t i = 0; i < _spatialStorageArea.size(); i++) {
      const auto& entry = _spatialStorageArea[i];
      const auto& entryId = std::get<1>(entry);

      // Set containing all areas we are inside of
      SkipSet skip;
      std::unordered_set<Area::id_t> skipByContainedInInner;

      const auto& queryResult = indexQryCover(entry);

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaArea = std::get<4>(area);
        const auto& areaFromType = std::get<5>(area);

        stats.checked();

        if (areaId == entryId) {
          continue;
        }

        if (areaFromType == AreaFromType::RELATION &&
            skipByContainedInInner.find(areaObjId) !=
                skipByContainedInInner.end()) {
          stats.skippedByContainedInInnerRing();
          continue;
        }

        if (skip.find(areaId) != skip.end()) {
          stats.skippedByDAG();
          continue;
        }

        GeomRelationInfo geomRelInf;
        if (!areaInAreaApprox(entry, area, &geomRelInf, &stats)) {
          continue;
        }

        if (areaFromType == AreaFromType::WAY) {
          // we are contained in an area derived from a way.
          const auto& relations = _areaBorderWaysIndex.find(areaObjId);
          if (relations != _areaBorderWaysIndex.end()) {
            for (auto r : relations->second) {
              if (r.second) {
                // way is inner geometry of this area relation, so if we
                // encounter the enclosing area again for this way, we can
                // be sure that we are not contained in it!
                skipByContainedInInner.insert(r.first);
              }
            }
          }
        }

        // skip equal geometries
        if (fabs(1 - areaArea / geomRelInf.intersectArea) < 0.05) {
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

    std::cerr << currentTimeFormatted() << " ... done.\n"
              << osm2rdf::util::formattedTimeSpacer << " checked "
              << stats.printTotalChecks() << " area pairs A, B\n"
              << osm2rdf::util::formattedTimeSpacer << " decided "
              << stats.printSkippedByDAG() << " by DAG\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByAreaSize() << " by area size\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByBox()
              << " by non-intersecting bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByOrientedBox()
              << " by non-intersecting oriented bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByBoxIdIntersect()
              << " by box id intersect\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByBoxIdIntersectCutout()
              << " by check against box id cutout\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByContainedInInnerRing()
              << " because A was in inner ring of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByConvexHull() << " by convex hull\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printFullChecks() << " by full geometric check"

              << std::endl;
  }
  if (_config.writeDAGDotFiles) {
    std::cerr << currentTimeFormatted() << " Dumping non-reduced DAG as "
              << _config.output << ".non-reduced.dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".non-reduced.dot";
    tmpDirectedAreaGraph.dump(p);
    std::cerr << currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " Reducing DAG with "
              << tmpDirectedAreaGraph.getNumEdges() << " edges and "
              << tmpDirectedAreaGraph.getNumVertices() << " vertices ... "
              << std::endl;

    // Prepare non-reduced DAG for cleanup
    tmpDirectedAreaGraph.prepareFindSuccessorsFast();
    std::cerr << currentTimeFormatted() << " ... fast lookup prepared ... "
              << std::endl;

    _directedAreaGraph = osm2rdf::util::reduceDAG(tmpDirectedAreaGraph, true);

    std::cerr << currentTimeFormatted() << " ... done, resulting in DAG with "
              << _directedAreaGraph.getNumEdges() << " edges and "
              << _directedAreaGraph.getNumVertices() << " vertices"
              << std::endl;
  }
  if (_config.writeDAGDotFiles) {
    std::cerr << currentTimeFormatted() << " Dumping DAG as " << _config.output
              << ".dot ..." << std::endl;
    std::filesystem::path p{_config.output};
    p += ".dot";
    _directedAreaGraph.dump(p);
    std::cerr << currentTimeFormatted() << " done" << std::endl;
  }
  {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted()
              << " Preparing fast above lookup in DAG ..." << std::endl;
    _directedAreaGraph.prepareFindSuccessorsFast();
    std::cerr << currentTimeFormatted() << " ... done" << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::dumpNamedAreaRelations() {
  std::cerr << std::endl;
  std::cerr << currentTimeFormatted() << " Dumping relations from DAG with "
            << _directedAreaGraph.getNumEdges() << " edges and "
            << _directedAreaGraph.getNumVertices()
            << " vertices and calculation intersect relationships... "
            << std::endl;

  osm2rdf::util::ProgressBar progressBar{_directedAreaGraph.getNumVertices(),
                                         true};
  size_t entryCount = 0;
  progressBar.update(entryCount);
  GeomRelationStats intersectStats;

  std::vector<DirectedGraph<Area::id_t>::entry_t> vertices =
      _directedAreaGraph.getVertices();
#pragma omp parallel for shared(                                       \
        vertices, osm2rdf::ttl::constants::NAMESPACE__OSM_WAY,         \
            osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,          \
            osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_AREA,       \
            osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,   \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA,     \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, \
            progressBar, entryCount)                                   \
    reduction(+ : intersectStats) default(none) schedule(static)
  for (size_t i = 0; i < vertices.size(); i++) {
    const auto id = vertices[i];
    const auto& entry = _spatialStorageArea[_spatialStorageAreaIndex[id]];
    const auto& entryId = std::get<1>(entry);
    const auto& entryObjId = std::get<3>(entry);
    const auto& entryFromType = std::get<5>(entry);
    std::string entryIRI =
        _writer->generateIRI(areaNS(entryFromType), entryObjId);

    // all areas we can skip via the DAG
    SkipSet skip;

    // contains relations, simply dump the DAG
    for (const auto& dst : _directedAreaGraph.getEdges(id)) {
      assert(_spatialStorageAreaIndex[dst] < _spatialStorageArea.size());
      const auto& area = _spatialStorageArea[_spatialStorageAreaIndex[dst]];
      const auto& areaId = std::get<1>(area);
      const auto& areaObjId = std::get<3>(area);
      const auto& areaFromType = std::get<5>(area);
      std::string areaIRI =
          _writer->generateIRI(areaNS(areaFromType), areaObjId);

      _writer->writeTriple(areaIRI, IRI__OSM2RDF_CONTAINS_AREA, entryIRI);
      _writer->writeTriple(areaIRI, IRI__OSM2RDF_INTERSECTS_AREA, entryIRI);
      _writer->writeTriple(entryIRI, IRI__OSM2RDF_INTERSECTS_AREA, areaIRI);

      // transitive closure
      const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);

      skip.insert(areaId);
      skip.insert(successors.begin(), successors.end());

      writeTransitiveClosure(successors, entryIRI, IRI__OSM2RDF_INTERSECTS_AREA,
                             IRI__OSM2RDF_INTERSECTS_AREA);
      writeTransitiveClosure(successors, entryIRI, IRI__OSM2RDF_CONTAINS_AREA);
    }

    // intersect relation, use R-Tree
    const auto& queryResult = indexQryIntersect(entry);

    for (const auto& areaRef : queryResult) {
      const auto& area = _spatialStorageArea[areaRef.second];
      const auto& areaId = std::get<1>(area);
      const auto& areaObjId = std::get<3>(area);
      const auto& areaFromType = std::get<5>(area);

      intersectStats.checked();

      // don't compare to itself
      if (areaId == entryId) {
        continue;
      }

      GeomRelationInfo geomRelInf;

      const auto& areaIRI =
          _writer->generateIRI(areaNS(areaFromType), areaObjId);

      if (skip.find(areaId) != skip.end()) {
        geomRelInf.intersects = RelInfoValue::YES;
        intersectStats.skippedByDAG();
      } else if (areaIntersectsArea(entry, area, &geomRelInf,
                                    &intersectStats)) {
        const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
        skip.insert(successors.begin(), successors.end());

        // transitive closure
        writeTransitiveClosure(successors, entryIRI,
                               IRI__OSM2RDF_INTERSECTS_AREA,
                               IRI__OSM2RDF_INTERSECTS_AREA);

        _writer->writeTriple(areaIRI, IRI__OSM2RDF_INTERSECTS_AREA, entryIRI);
        _writer->writeTriple(entryIRI, IRI__OSM2RDF_INTERSECTS_AREA, areaIRI);
      }
    }

#pragma omp critical(progress)
    progressBar.update(entryCount++);
  }

  progressBar.done();

  std::cerr << currentTimeFormatted() << " ... done.\n"
            << osm2rdf::util::formattedTimeSpacer << " checked "
            << intersectStats.printTotalChecks()
            << " area/area pairs A, B for intersect\n"
            << osm2rdf::util::formattedTimeSpacer << " decided "
            << intersectStats.printSkippedByDAG() << " by DAG\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBox()
            << " by non-intersecting bounding boxes \n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByOrientedBox()
            << " by non-intersecting oriented bounding boxes \n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBoxIdIntersect()
            << " by box id intersect\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBoxIdIntersectCutout()
            << " by check against box id cutout\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByInner()
            << " by inner simplified geom\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByOuter()
            << " by outer simplified geom\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printFullChecks() << " by full geometric check"
            << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::dumpUnnamedAreaRelations() {
  if (_config.noAreaGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Skipping contains relation for unnamed areas ... disabled"
              << std::endl;
  } else if (_numUnnamedAreas == 0) {
    std::cerr << std::endl;
    std::cerr
        << currentTimeFormatted() << " "
        << "Skipping contains relation for unnamed areas ... no unnamed area"
        << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Contains relations for " << _numUnnamedAreas
              << " unnamed areas in " << _spatialIndex.size() << " areas ..."
              << std::endl;

    // reset stream
    _fsUnnamedAreas.clear();
    _fsUnnamedAreas.seekg(0, std::ios::beg);
    boost::archive::binary_iarchive ia(_fsUnnamedAreas);

    osm2rdf::util::ProgressBar progressBar{_numUnnamedAreas, true};
    GeomRelationStats intersectStats;
    GeomRelationStats containsStats;
    size_t entryCount = 0;
    progressBar.update(entryCount);
#pragma omp parallel for shared(                                       \
        osm2rdf::ttl::constants::NAMESPACE__OSM_WAY,                   \
            osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,          \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA, \
            osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,   \
            progressBar, entryCount, ia)                               \
    reduction(+ : intersectStats, containsStats) default(none)         \
    schedule(dynamic)
    for (size_t i = 0; i < _numUnnamedAreas; i++) {
      SpatialAreaValue entry;
#pragma omp critical(loadEntry)
      ia >> entry;

      const auto& entryId = std::get<1>(entry);
      const auto& entryObjId = std::get<3>(entry);
      const auto& entryFromType = std::get<5>(entry);
      std::string entryIRI =
          _writer->generateIRI(areaNS(entryFromType), entryObjId);

      // Set containing all areas we are inside of
      SkipSet skipIntersects;
      SkipSet skipContains;

      const auto& queryResult = indexQryIntersect(entry);

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromType = std::get<5>(area);

        intersectStats.checked();
        containsStats.checked();

        // don't compare to itself
        if (areaId == entryId) {
          continue;
        }

        GeomRelationInfo geomRelInf;

        const auto& areaIRI =
            _writer->generateIRI(areaNS(areaFromType), areaObjId);

        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          geomRelInf.intersects = RelInfoValue::YES;
          intersectStats.skippedByDAG();
        } else if (areaIntersectsArea(entry, area, &geomRelInf,
                                      &intersectStats)) {
          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          // transitive closure
          writeTransitiveClosure(successors, entryIRI,
                                 IRI__OSM2RDF_INTERSECTS_NON_AREA,
                                 IRI__OSM2RDF_INTERSECTS_NON_AREA);

          _writer->writeTriple(areaIRI, IRI__OSM2RDF_INTERSECTS_NON_AREA,
                               entryIRI);
          _writer->writeTriple(entryIRI, IRI__OSM2RDF_INTERSECTS_NON_AREA,
                               areaIRI);
        }

        if (geomRelInf.intersects == RelInfoValue::NO) {
          containsStats.skippedByNonIntersect();
          continue;
        }

        if (skipContains.find(areaId) != skipContains.end()) {
          containsStats.skippedByDAG();
        } else {
          if (areaInArea(entry, area, &geomRelInf, &containsStats)) {
            const auto& successors =
                _directedAreaGraph.findSuccessorsFast(areaId);
            skipContains.insert(successors.begin(), successors.end());

            // transitive closure
            writeTransitiveClosure(successors, entryIRI,
                                   IRI__OSM2RDF_CONTAINS_NON_AREA);

            _writer->writeTriple(areaIRI, IRI__OSM2RDF_CONTAINS_NON_AREA,
                                 entryIRI);
          }
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << currentTimeFormatted() << " ... done.\n"
              << osm2rdf::util::formattedTimeSpacer << " checked "
              << intersectStats.printTotalChecks()
              << " area/area pairs A, B for intersect\n"
              << osm2rdf::util::formattedTimeSpacer << " decided "
              << intersectStats.printSkippedByDAG() << " by DAG\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBox()
              << " by non-intersecting bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByOrientedBox()
              << " by non-intersecting oriented bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBoxIdIntersect()
              << " by box id intersect\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBoxIdIntersectCutout()
              << " by check against box id cutout\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByInner()
              << " by inner simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByOuter()
              << " by outer simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printFullChecks()
              << " by full geometric check\n\n"

              << osm2rdf::util::formattedTimeSpacer << " checked "
              << containsStats.printTotalChecks()
              << " area/area pairs A, B for contains\n"
              << osm2rdf::util::formattedTimeSpacer << " decided "
              << containsStats.printSkippedByDAG() << " by DAG\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByNonIntersect()
              << " because A did not intersect B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByAreaSize() << " by area size\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByBox()
              << " by non-containing bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByBoxIdIntersect()
              << " by box id intersect\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByBoxIdIntersectCutout()
              << " by check against box id cutout\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByContainedInInnerRing()
              << " because A was in inner ring of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByInner()
              << " by inner simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByOuter()
              << " by outer simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByConvexHull() << " by convex hull\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByOrientedBox()
              << " by oriented bounding box\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printFullChecks() << " by full geometric check"

              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << (static_cast<double>(intersectStats._totalChecks) /
                  _numUnnamedAreas)
              << " areas checked per geometry on average" << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::NodesContainedInAreasData
GeometryHandler<W>::dumpNodeRelations() {
  // Store for each node all relevant areas
  NodesContainedInAreasData nodeData;
  if (_config.noNodeGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Skipping contains relation for nodes ... disabled"
              << std::endl;
  } else if (_numNodes == 0) {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Skipping contains relation for nodes ... no nodes"
              << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Contains relations for " << _numNodes << " nodes in "
              << _spatialIndex.size() << " areas ..." << std::endl;

    _fsNodes.clear();
    _fsNodes.seekg(0, std::ios::beg);
    boost::archive::binary_iarchive ia(_fsNodes);

    osm2rdf::util::ProgressBar progressBar{_numNodes, true};
    size_t entryCount = 0;

    GeomRelationStats stats;

    progressBar.update(entryCount);

#pragma omp parallel for shared(                                             \
        std::cout, osm2rdf::ttl::constants::NAMESPACE__OSM_NODE,             \
            osm2rdf::ttl::constants::NAMESPACE__OSM_WAY,                     \
            osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,                \
            osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,         \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,       \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA, nodeData, \
            progressBar, ia, entryCount) reduction(+ : stats) default(none)  \
    schedule(dynamic)
    for (size_t i = 0; i < _numNodes; i++) {
      SpatialNodeValue node;
#pragma omp critical(loadEntry)
      ia >> node;

      const auto& nodeId = std::get<0>(node);
      std::string nodeIRI = _writer->generateIRI(NAMESPACE__OSM_NODE, nodeId);

      // Set containing all areas we are inside of
      SkipSet skip;
      std::unordered_set<Area::id_t> skipByContainedInInner;

      const auto& queryResult = indexQry(node);

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromType = std::get<5>(area);

        stats.checked();

        if (areaFromType == AreaFromType::RELATION &&
            skipByContainedInInner.find(areaObjId) !=
                skipByContainedInInner.end()) {
          stats.skippedByContainedInInnerRing();
          continue;
        }

        if (skip.find(areaId) != skip.end()) {
          stats.skippedByDAG();
          continue;
        }

        GeomRelationInfo geomRelInf;
        if (!nodeInArea(node, area, &geomRelInf, &stats)) {
          continue;
        }

        if (areaFromType == AreaFromType::WAY) {
          // we are contained in an area derived from a way.
          const auto& relations = _areaBorderWaysIndex.find(areaObjId);
          if (relations != _areaBorderWaysIndex.end()) {
            for (auto r : relations->second) {
              if (r.second) {
                // way is inner geometry of this area relation, so if we
                // encounter the enclosing area again for this way, we can
                // be sure that we are not contained in it!
                skipByContainedInInner.insert(r.first);
              }
            }
          }
        }

        skip.insert(areaId);

        const auto& successors = _directedAreaGraph.findSuccessorsFast(areaId);
        skip.insert(successors.begin(), successors.end());

        std::string areaIRI =
            _writer->generateIRI(areaNS(areaFromType), areaObjId);

        // transitive closure
        writeTransitiveClosure(successors, nodeIRI,
                               IRI__OSM2RDF_INTERSECTS_NON_AREA,
                               IRI__OSM2RDF_INTERSECTS_AREA);
        writeTransitiveClosure(successors, nodeIRI,
                               IRI__OSM2RDF_CONTAINS_NON_AREA);

        _writer->writeTriple(
            areaIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,
            nodeIRI);
        _writer->writeTriple(
            nodeIRI, osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA,
            areaIRI);
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

    std::cerr << currentTimeFormatted() << " ... done.\n"
              << osm2rdf::util::formattedTimeSpacer << " checked "
              << stats.printTotalChecks()
              << " node/area pairs A, B for intersect\n"
              << osm2rdf::util::formattedTimeSpacer << " decided "
              << stats.printSkippedByDAG() << " by DAG\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByOrientedBox()
              << " because A was no in OBB of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByBoxIdIntersect()
              << " by box id intersect\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByBoxIdIntersectCutout()
              << " by check against box id cutout\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByContainedInInnerRing()
              << " because A was in inner ring of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByInner() << " by inner simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printSkippedByOuter() << " by outer simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << stats.printFullChecks() << " by full geometric check"
              << std::endl;

    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << (static_cast<double>(stats._totalChecks) / _numWays)
              << " areas checked per geometry on average" << std::endl;
  }
  return nodeData;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::dumpWayWayRelations() {
  std::cerr << std::endl;
  std::cerr << currentTimeFormatted() << " "
            << "Way/way relations for " << _numWays << " ways in "
            << _spatialWayIndex.size() << " ways ..." << std::endl;

  _fsWays.clear();
  _fsWays.seekg(0, std::ios::beg);
  boost::archive::binary_iarchive ia(_fsWays);

  osm2rdf::util::ProgressBar progressBar{_numWays, true};
  size_t entryCount = 0;

  GeomRelationStats intersectStats;
  GeomRelationStats containsStats;

  progressBar.update(entryCount);
#pragma omp parallel for shared(                                           \
      std::cout, std::cerr, osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, \
          osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,    \
          osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,     \
          osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA,         \
          osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,       \
          progressBar, entryCount, ia)                                   \
  reduction(+ : intersectStats, containsStats) default(none)             \
  schedule(dynamic)

  for (size_t i = 0; i < _numWays; i++) {
    SpatialWayValue way;

#pragma omp critical(loadEntry)
    ia >> way;

    const auto& wayId = std::get<1>(way);
    const auto& wayNodeIds = std::get<3>(way);

    std::string wayIRI = _writer->generateIRI(NAMESPACE__OSM_WAY, wayId);

    // Set containing all areas we are inside of
    SkipSet skipNodeContained;
    SkipSet skipIntersects;
    SkipSet skipContains;

    const auto& queryResult = wayIndexQryIntersect(way);

    for (const auto& wayRef : queryResult) {
      const auto& idxWay = _spatialStorageWay[wayRef.second];
      const auto& idxWayId = std::get<1>(idxWay);
      const auto& idxWayNodeIds = std::get<3>(idxWay);

      intersectStats.checked();
      containsStats.checked();

      GeomRelationInfo geomRelInf;

      // checks for intersect
      if (wayIntersectsWay(way, idxWay, &geomRelInf, &intersectStats)) {
        std::string idxWayIRI = _writer->generateIRI(NAMESPACE__OSM_WAY, idxWayId);

        _writer->writeTriple(idxWayIRI, IRI__OSM2RDF_INTERSECTS_NON_AREA,
                             wayIRI);
      }

      if (geomRelInf.intersects == RelInfoValue::NO) {
        containsStats.skippedByNonIntersect();
        continue;
      }

      // checks for contains
      if (!wayInWay(way, idxWay, &geomRelInf, &containsStats)) {
        continue;
      }

      std::string idxWayIRI = _writer->generateIRI(NAMESPACE__OSM_WAY, idxWayId);

      _writer->writeTriple(idxWayIRI, IRI__OSM2RDF_CONTAINS_NON_AREA, wayIRI);
    }
#pragma omp critical(progress)
    progressBar.update(entryCount++);
  }
  progressBar.done();

  std::cerr << currentTimeFormatted() << " ... done.\n"
            << osm2rdf::util::formattedTimeSpacer << " checked "
            << intersectStats.printTotalChecks()
            << " way/way pairs A, B for intersect\n"
            << osm2rdf::util::formattedTimeSpacer << " decided "
            << intersectStats.printSkippedByDAG() << " by DAG\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBorderContained()
            << " because A was part of B's ring\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByNodeContained()
            << " because a node of A is in a "
               "ring of B\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBox()
            << " by non-intersecting bounding boxes \n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByOrientedBox()
            << " by oriented bounding-boxes \n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBoxIdIntersect()
            << " by box id intersect\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByBoxIdIntersectCutout()
            << " by check against box id cutout\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByContainedInInnerRing()
            << " because A was in inner ring of B\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByInner()
            << " by inner simplified geom\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printSkippedByOuter()
            << " by outer simplified geom\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << intersectStats.printFullChecks()
            << " by full geometric check\n\n"

            << osm2rdf::util::formattedTimeSpacer << " checked "
            << containsStats.printTotalChecks()
            << " way/way pairs A, B for contains\n"
            << osm2rdf::util::formattedTimeSpacer << " decided "
            << containsStats.printSkippedByDAG() << " by DAG\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByNonIntersect()
            << " because A did not intersect B\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByBox()
            << " by non-contained bounding boxes \n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByBoxIdIntersect()
            << " by box id intersect\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByBoxIdIntersectCutout()
            << " by check against box id cutout\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByNodeContained()
            << " because A was node-contained in B\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByInner()
            << " by inner simplified geom\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByOuter()
            << " by outer simplified geom\n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printSkippedByOrientedBox()
            << " by oriented bounding-box \n"
            << osm2rdf::util::formattedTimeSpacer << "         "
            << containsStats.printFullChecks() << " by full geometric check"

            << std::endl;
  std::cerr << osm2rdf::util::formattedTimeSpacer << " "
            << (static_cast<double>(intersectStats._totalChecks) / _numWays)
            << " areas checked per geometry on average" << std::endl;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::dumpWayRelations(
    const NodesContainedInAreasData& nodeData) {
  if (_config.noWayGeometricRelations) {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Skipping contains relation for ways ... disabled"
              << std::endl;
  } else if (_numWays == 0) {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Skipping contains relation for ways ... no ways" << std::endl;
  } else {
    std::cerr << std::endl;
    std::cerr << currentTimeFormatted() << " "
              << "Contains relations for " << _numWays << " ways in "
              << _spatialIndex.size() << " areas ..." << std::endl;

    _fsWays.clear();
    _fsWays.seekg(0, std::ios::beg);
    boost::archive::binary_iarchive ia(_fsWays);

    osm2rdf::util::ProgressBar progressBar{_numWays, true};
    size_t entryCount = 0;

    GeomRelationStats intersectStats;
    GeomRelationStats containsStats;

    progressBar.update(entryCount);
#pragma omp parallel for shared(                                           \
        std::cout, std::cerr, osm2rdf::ttl::constants::NAMESPACE__OSM_WAY, \
            nodeData, osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION,    \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_NON_AREA,     \
            osm2rdf::ttl::constants::IRI__OSM2RDF_INTERSECTS_AREA,         \
            osm2rdf::ttl::constants::IRI__OSM2RDF_CONTAINS_NON_AREA,       \
            progressBar, entryCount, ia)                                   \
    reduction(+ : intersectStats, containsStats) default(none)             \
    schedule(dynamic)

    for (size_t i = 0; i < _numWays; i++) {
      SpatialWayValue way;

#pragma omp critical(loadEntry)
      ia >> way;

      const auto& wayId = std::get<1>(way);
      const auto& wayNodeIds = std::get<3>(way);

      // Check if our "area" has successors in the DAG, if yes we are part of
      // the DAG and don't need to calculate any relation again.
      if (!_directedAreaGraph.findSuccessorsFast(wayId * 2).empty()) {
        continue;
      }

      std::string wayIRI = _writer->generateIRI(NAMESPACE__OSM_WAY, wayId);

      // Set containing all areas we are inside of
      SkipSet skipNodeContained;
      SkipSet skipIntersects;
      SkipSet skipContains;
      std::unordered_set<Area::id_t> skipByContainedInInner;

      // Store known areas in set.
      for (const auto& nodeId : wayNodeIds) {
        auto nodeDataIt = nodeData.find(nodeId);
        if (nodeDataIt == nodeData.end()) {
          continue;
        }

        skipNodeContained.insert(nodeDataIt->second.begin(),
                                 nodeDataIt->second.end());
      }

      const auto& queryResult = indexQryIntersect(way);

      for (const auto& areaRef : queryResult) {
        const auto& area = _spatialStorageArea[areaRef.second];
        const auto& areaId = std::get<1>(area);
        const auto& areaObjId = std::get<3>(area);
        const auto& areaFromType = std::get<5>(area);

        intersectStats.checked();
        containsStats.checked();

        if (areaFromType == AreaFromType::RELATION &&
            skipByContainedInInner.find(areaObjId) !=
                skipByContainedInInner.end()) {
          intersectStats.skippedByContainedInInnerRing();
          containsStats.skippedByContainedInInnerRing();
          continue;
        }

        if (areaFromType == AreaFromType::WAY && areaObjId == wayId) {
          continue;
        }

        GeomRelationInfo geomRelInf;

        // checks for intersect

        if (skipIntersects.find(areaId) != skipIntersects.end()) {
          intersectStats.skippedByDAG();
          geomRelInf.intersects = RelInfoValue::YES;
        } else if (areaFromType == AreaFromType::RELATION &&
                   borderContained(wayId, areaObjId)) {
          intersectStats.skippedByBorderContained();
          containsStats.skippedByBorderContained();

          geomRelInf.intersects = RelInfoValue::YES;
          geomRelInf.contained = RelInfoValue::YES;

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNS(areaFromType), areaObjId);

          // transitive closure
          writeTransitiveClosure(successors, wayIRI,
                                 IRI__OSM2RDF_INTERSECTS_NON_AREA,
                                 IRI__OSM2RDF_INTERSECTS_AREA);

          _writer->writeTriple(areaIRI, IRI__OSM2RDF_INTERSECTS_NON_AREA,
                               wayIRI);
          _writer->writeTriple(wayIRI, IRI__OSM2RDF_INTERSECTS_AREA, areaIRI);
        } else if (skipNodeContained.find(areaId) != skipNodeContained.end()) {
          intersectStats.skippedByNodeContained();
          geomRelInf.intersects = RelInfoValue::YES;

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNS(areaFromType), areaObjId);

          // transitive closure
          writeTransitiveClosure(successors, wayIRI,
                                 IRI__OSM2RDF_INTERSECTS_NON_AREA,
                                 IRI__OSM2RDF_INTERSECTS_AREA);

          _writer->writeTriple(areaIRI, IRI__OSM2RDF_INTERSECTS_NON_AREA,
                               wayIRI);
          _writer->writeTriple(wayIRI, IRI__OSM2RDF_INTERSECTS_AREA, areaIRI);
        } else if (wayIntersectsArea(way, area, &geomRelInf, &intersectStats)) {
          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipIntersects.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNS(areaFromType), areaObjId);

          // transitive closure
          writeTransitiveClosure(successors, wayIRI,
                                 IRI__OSM2RDF_INTERSECTS_NON_AREA,
                                 IRI__OSM2RDF_INTERSECTS_AREA);

          _writer->writeTriple(areaIRI, IRI__OSM2RDF_INTERSECTS_NON_AREA,
                               wayIRI);
          _writer->writeTriple(wayIRI, IRI__OSM2RDF_INTERSECTS_AREA, areaIRI);
        }

        if (geomRelInf.intersects == RelInfoValue::NO) {
          containsStats.skippedByNonIntersect();
          continue;
        }

        // checks for contains

        if (skipContains.find(areaId) != skipContains.end()) {
          containsStats.skippedByDAG();
        } else {
          if (!wayInArea(way, area, &geomRelInf, &containsStats)) {
            continue;
          }

          if (areaFromType == AreaFromType::WAY) {
            // we are contained in an area derived from a way.
            const auto& relations = _areaBorderWaysIndex.find(areaObjId);
            if (relations != _areaBorderWaysIndex.end()) {
              for (auto r : relations->second) {
                if (r.second) {
                  // way is inner geometry of this area relation, so if we
                  // encounter the enclosing area again for this way, we can
                  // be sure that we are not contained in it!
                  skipByContainedInInner.insert(r.first);
                }
              }
            }
          }

          const auto& successors =
              _directedAreaGraph.findSuccessorsFast(areaId);
          skipContains.insert(successors.begin(), successors.end());

          std::string areaIRI =
              _writer->generateIRI(areaNS(areaFromType), areaObjId);

          // transitive closure
          writeTransitiveClosure(successors, wayIRI,
                                 IRI__OSM2RDF_CONTAINS_NON_AREA);

          _writer->writeTriple(areaIRI, IRI__OSM2RDF_CONTAINS_NON_AREA, wayIRI);
        }
      }
#pragma omp critical(progress)
      progressBar.update(entryCount++);
    }
    progressBar.done();

    std::cerr << currentTimeFormatted() << " ... done.\n"
              << osm2rdf::util::formattedTimeSpacer << " checked "
              << intersectStats.printTotalChecks()
              << " way/area pairs A, B for intersect\n"
              << osm2rdf::util::formattedTimeSpacer << " decided "
              << intersectStats.printSkippedByDAG() << " by DAG\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBorderContained()
              << " because A was part of B's ring\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByNodeContained()
              << " because a node of A is in a "
                 "ring of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBox()
              << " by non-intersecting bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByOrientedBox()
              << " by oriented bounding-boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBoxIdIntersect()
              << " by box id intersect\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByBoxIdIntersectCutout()
              << " by check against box id cutout\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByContainedInInnerRing()
              << " because A was in inner ring of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByInner()
              << " by inner simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printSkippedByOuter()
              << " by outer simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << intersectStats.printFullChecks()
              << " by full geometric check\n\n"

              << osm2rdf::util::formattedTimeSpacer << " checked "
              << containsStats.printTotalChecks()
              << " way/area pairs A, B for contains\n"
              << osm2rdf::util::formattedTimeSpacer << " decided "
              << containsStats.printSkippedByDAG() << " by DAG\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByNonIntersect()
              << " because A did not intersect B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByBox()
              << " by non-contained bounding boxes \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByBoxIdIntersect()
              << " by box id intersect\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByBoxIdIntersectCutout()
              << " by check against box id cutout\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByContainedInInnerRing()
              << " because A was in inner ring of B\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByInner()
              << " by inner simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByOuter()
              << " by outer simplified geom\n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printSkippedByOrientedBox()
              << " by oriented bounding-box \n"
              << osm2rdf::util::formattedTimeSpacer << "         "
              << containsStats.printFullChecks() << " by full geometric check"

              << std::endl;
    std::cerr << osm2rdf::util::formattedTimeSpacer << " "
              << (static_cast<double>(intersectStats._totalChecks) / _numWays)
              << " areas checked per geometry on average" << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::nodeInArea(const SpatialNodeValue& a,
                                    const SpatialAreaValue& b,
                                    GeomRelationInfo* geomRelInf,
                                    GeomRelationStats* stats) const {
  const auto& geomA = std::get<1>(a);
  int32_t ndBoxId = getBoxId(geomA);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& areaBoxIds = std::get<8>(b);
  const auto& areaCutouts = std::get<9>(b);
  const auto& areaObb = std::get<11>(b);

  if (!boost::geometry::intersects(geomA, areaObb)) {
    // not in oriented bounding box
    geomRelInf->intersects = RelInfoValue::NO;
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  if (geomRelInf->fullContained < 0) {
    boxIdIsect({{1, 0}, {ndBoxId, 0}}, areaBoxIds, geomRelInf);
  }

  if (geomRelInf->fullContained > 0) {
    geomRelInf->intersects = RelInfoValue::YES;
    geomRelInf->contained = RelInfoValue::YES;
    stats->skippedByBoxIdIntersect();
    return true;
  }

  if (geomRelInf->toCheck.empty()) {
    geomRelInf->intersects = RelInfoValue::NO;
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByBoxIdIntersect();
    return false;
  }

  if (!areaCutouts.empty()) {
    stats->skippedByBoxIdIntersectCutout();
    for (auto boxId : geomRelInf->toCheck) {
      const auto& cutout = areaCutouts.find(boxId);

      if (cutout != areaCutouts.end()) {
        if (boost::geometry::intersects(geomA, cutout->second)) {
          geomRelInf->intersects = RelInfoValue::YES;
          geomRelInf->contained = RelInfoValue::YES;
          return true;
        }
      }
    }
    geomRelInf->intersects = RelInfoValue::NO;
    geomRelInf->contained = RelInfoValue::NO;
    return false;
  }

  if (!(_config.geometryOptimizations &
        osm2rdf::config::GeometryRelationOptimization::APPROXIMATE_POLYGONS) ||
      boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    stats->fullCheck();
    if (boost::geometry::covered_by(geomA, geomB)) {
      geomRelInf->intersects = RelInfoValue::YES;
      geomRelInf->contained = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->intersects = RelInfoValue::NO;
      geomRelInf->contained = RelInfoValue::NO;
      return false;
    }
  }

  if (false) {
    stats->skippedByOuter();
    if (boost::geometry::covered_by(geomA, outerGeomB)) {
      geomRelInf->intersects = RelInfoValue::YES;
      geomRelInf->contained = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->intersects = RelInfoValue::NO;
      geomRelInf->contained = RelInfoValue::NO;
      return false;
    }
  }

  if (boost::geometry::covered_by(geomA, innerGeomB)) {
    // if covered by simplified inner, we are definitely contained
    stats->skippedByInner();
    return true;
  }

  if (!boost::geometry::covered_by(geomA, outerGeomB)) {
    // if NOT covered by simplified out, we are definitely not contained
    stats->skippedByOuter();
    return false;
  }

  stats->fullCheck();

  if (boost::geometry::covered_by(geomA, geomB)) {
    return true;
  }

  return false;
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::areaIntersectsArea(const SpatialAreaValue& a,
                                            const SpatialAreaValue& b,
                                            GeomRelationInfo* geomRelInf,
                                            GeomRelationStats* stats) const {
  if (geomRelInf->intersects == RelInfoValue::YES) {
    return true;
  }

  const auto& geomA = std::get<2>(a);
  const auto& geomB = std::get<2>(b);
  const auto& innerGeomA = std::get<6>(a);
  const auto& outerGeomA = std::get<7>(a);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& boxIdsA = std::get<8>(a);
  const auto& boxIdsB = std::get<8>(b);
  const auto& cutoutsA = std::get<9>(a);
  const auto& cutoutsB = std::get<9>(b);
  const auto& obbA = std::get<11>(a);
  const auto& obbB = std::get<11>(b);

  if (_config.geometryOptimizations &
          osm2rdf::config::GeometryRelationOptimization::
              OBJECT_ORIENTED_BOUNDING_BOX &&
      !boost::geometry::intersects(obbA, obbB)) {
    // ... oriented bounding boxes do no intersect
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  // if no geometric relation has been written so far, do it now
  if (geomRelInf->fullContained < 0) boxIdIsect(boxIdsA, boxIdsB, geomRelInf);

  // if there is at least one full contained box, we surely intersect
  if (geomRelInf->fullContained > 0) {
    geomRelInf->intersects = RelInfoValue::YES;
    stats->skippedByBoxIdIntersect();
    return true;
  }

  // if there is no full contained box, and no potentially contained, we
  // surely do not intersect
  if (geomRelInf->toCheck.empty()) {
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByBoxIdIntersect();
    return false;
  }

  // if we have cutouts for b or a, use them to check for intersection
  if (!cutoutsA.empty() || !cutoutsB.empty()) {
    stats->skippedByBoxIdIntersectCutout();
    for (auto boxId : geomRelInf->toCheck) {
      const auto& cutoutA = cutoutsA.find(boxId);
      const auto& cutoutB = cutoutsB.find(boxId);

      if (cutoutA != cutoutsA.end() && cutoutB != cutoutsB.end()) {
        if (boost::geometry::intersects(cutoutA->second, cutoutB->second)) {
          geomRelInf->intersects = RelInfoValue::YES;
          return true;
        }
      }

      if (cutoutA != cutoutsA.end()) {
        if (boost::geometry::intersects(cutoutA->second, geomB)) {
          geomRelInf->intersects = RelInfoValue::YES;
          return true;
        }
      }

      if (cutoutB != cutoutsB.end()) {
        if (boost::geometry::intersects(geomA, cutoutB->second)) {
          geomRelInf->intersects = RelInfoValue::YES;
          return true;
        }
      }
    }

    geomRelInf->intersects = RelInfoValue::NO;
    return false;
  }

  if (!(_config.geometryOptimizations &
        osm2rdf::config::GeometryRelationOptimization::APPROXIMATE_POLYGONS) ||
      boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    stats->fullCheck();
    if (boost::geometry::intersects(geomA, geomB)) {
      geomRelInf->intersects = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->intersects = RelInfoValue::NO;
      return false;
    }
  }

  if (boost::geometry::intersects(innerGeomA, innerGeomB)) {
    // if simplified inner intersect, we definitely intersect
    geomRelInf->intersects = RelInfoValue::YES;
    stats->skippedByInner();
    return true;
  }

  if (!boost::geometry::intersects(outerGeomA, outerGeomB)) {
    // if NOT intersecting simplified outer, we are definitely NOT
    // intersecting
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOuter();
    return false;
  }

  stats->fullCheck();

  if (boost::geometry::intersects(geomA, geomB)) {
    geomRelInf->intersects = RelInfoValue::YES;
    return true;
  }

  geomRelInf->intersects = RelInfoValue::NO;
  return false;
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::wayInWay(const SpatialWayValue& a,
                                           const SpatialWayValue& b,
                                           GeomRelationInfo* geomRelInf,
                                           GeomRelationStats* stats) const {
  // shortcut
  if (geomRelInf->contained == RelInfoValue::YES) {
    return true;
  }

  //TODO: use node ids here for positive checks

  const auto& geomA = std::get<2>(a);
  const auto& wayANodeIds = std::get<3>(a);
  const auto& wayABoxIds = std::get<5>(a);
  const auto& wayAOBB = std::get<7>(a);

  const auto& geomB = std::get<2>(b);
  const auto& wayBNodeIds = std::get<3>(b);
  const auto& wayBBoxIds = std::get<5>(b);
  const auto& wayBOBB = std::get<7>(b);

  // if we haven't intersected the box ids yet, do it now
  if (geomRelInf->fullContained < 0)
    boxIdIsect(wayABoxIds, wayBBoxIds, geomRelInf);

  // if we have no potential intersection box, we do not
  // intersect
  // full contained means "shares box" in the context of ways!
  if (geomRelInf->fullContained == 0) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByBoxIdIntersect();
    return false;
  }

  if (wayANodeIds.size() > 0) {
    size_t i = 0;
    for (size_t j = 0; j < wayBNodeIds.size(); j++) {
      if (wayBNodeIds[j] == wayANodeIds[i]) {
        if (wayBNodeIds.size() < j + wayANodeIds.size()) break;

        bool cont = true;

        for (; i < wayANodeIds.size(); i++,j++) {
          if (wayANodeIds[i] != wayBNodeIds[j]) {
            cont = false;
            break;
          }
        }

        if (cont) {
          geomRelInf->contained = RelInfoValue::YES;
          stats->skippedByNodeContained();
          return false;
        }

      }
    }
  }

  // if we have potential intersection boxes, and areaCutouts to use,
  // only check against them
  // TODO!
  // if (!areaCutouts.empty()) {
    // for (auto boxId : geomRelInf->toCheck) {
      // const auto& cutout = areaCutouts.find(boxId);

      // if (cutout != areaCutouts.end()) {
        // if (boost::geometry::intersects(geomA, cutout->second)) {
          // geomRelInf->intersects = RelInfoValue::YES;
          // stats->skippedByBoxIdIntersectCutout();
          // return true;
        // }
      // }
    // }
    // stats->skippedByBoxIdIntersectCutout();
    // geomRelInf->intersects = RelInfoValue::NO;
    // return false;
  // }

  if (!boost::geometry::covered_by(geomA, wayBOBB)) {
    // ... does not intersect with oriented bounding box
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  stats->fullCheck();
  if (boost::geometry::covered_by(geomA, geomB)) {
    geomRelInf->contained = RelInfoValue::YES;
    return true;
  }

  geomRelInf->contained = RelInfoValue::NO;
  return false;
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::wayIntersectsWay(const SpatialWayValue& a,
                                           const SpatialWayValue& b,
                                           GeomRelationInfo* geomRelInf,
                                           GeomRelationStats* stats) const {
  // shortcut
  if (geomRelInf->intersects == RelInfoValue::YES) {
    return true;
  }

  //TODO: use node ids here for positive checks

  const auto& geomA = std::get<2>(a);
  // const auto& wayANodeIds = std::get<3>(a);
  const auto& wayABoxIds = std::get<5>(a);
  const auto& wayAOBB = std::get<7>(a);

  const auto& geomB = std::get<2>(b);
  // const auto& wayBNodeIds = std::get<3>(b);
  const auto& wayBBoxIds = std::get<5>(b);
  const auto& wayBOBB = std::get<7>(b);

  if (!boost::geometry::intersects(wayAOBB, wayBOBB)) {
    // ... does not intersect with oriented bounding box
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  // if we haven't intersected the box ids yet, do it now
  if (geomRelInf->fullContained < 0)
    boxIdIsect(wayABoxIds, wayBBoxIds, geomRelInf);

  // if we have no potential intersection box, we do not
  // intersect
  // full contained means "shares box" in the context of ways!
  if (geomRelInf->fullContained == 0) {
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByBoxIdIntersect();
    return false;
  }

  // if we have potential intersection boxes, and areaCutouts to use,
  // only check against them
  // TODO!
  // if (!areaCutouts.empty()) {
    // for (auto boxId : geomRelInf->toCheck) {
      // const auto& cutout = areaCutouts.find(boxId);

      // if (cutout != areaCutouts.end()) {
        // if (boost::geometry::intersects(geomA, cutout->second)) {
          // geomRelInf->intersects = RelInfoValue::YES;
          // stats->skippedByBoxIdIntersectCutout();
          // return true;
        // }
      // }
    // }
    // stats->skippedByBoxIdIntersectCutout();
    // geomRelInf->intersects = RelInfoValue::NO;
    // return false;
  // }

  if (!boost::geometry::intersects(geomA, wayBOBB)) {
    // ... does not intersect with oriented bounding box
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  stats->fullCheck();
  if (boost::geometry::intersects(geomA, geomB)) {
    geomRelInf->intersects = RelInfoValue::YES;
    return true;
  }

  geomRelInf->intersects = RelInfoValue::NO;
  return false;
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::wayIntersectsArea(const SpatialWayValue& a,
                                           const SpatialAreaValue& b,
                                           GeomRelationInfo* geomRelInf,
                                           GeomRelationStats* stats) const {
  // shortcut
  if (geomRelInf->intersects == RelInfoValue::YES) {
    return true;
  }

  const auto& geomA = std::get<2>(a);
  const auto& wayBoxIds = std::get<5>(a);
  const auto& wayOBB = std::get<7>(a);

  const auto& geomB = std::get<2>(b);
  const auto& envelopesB = std::get<0>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& areaBoxIds = std::get<8>(b);
  const auto& areaCutouts = std::get<9>(b);
  // const auto& areaConvexHull = std::get<10>(b);
  const auto& areaOBB = std::get<11>(b);

  if (!boost::geometry::intersects(wayOBB, areaOBB)) {
    // ... does not intersect with oriented bounding box
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  // if we haven't intersected the box ids yet, do it now
  if (geomRelInf->fullContained < 0)
    boxIdIsect(wayBoxIds, areaBoxIds, geomRelInf);

  // if we have at least one of A's boxes fully contained in B, we surely
  // intersect
  if (geomRelInf->fullContained > 0) {
    geomRelInf->intersects = RelInfoValue::YES;
    stats->skippedByBoxIdIntersect();
    return true;
  }

  // if not, and if we also have no potential intersection box, we are not
  // contained
  if (geomRelInf->toCheck.empty()) {
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByBoxIdIntersect();
    return false;
  }

  // if we have potential intersection boxes, and areaCutouts to use,
  // only check against them
  if (!areaCutouts.empty()) {
    for (auto boxId : geomRelInf->toCheck) {
      const auto& cutout = areaCutouts.find(boxId);

      if (cutout != areaCutouts.end()) {
        if (boost::geometry::intersects(geomA, cutout->second)) {
          geomRelInf->intersects = RelInfoValue::YES;
          stats->skippedByBoxIdIntersectCutout();
          return true;
        }
      }
    }
    stats->skippedByBoxIdIntersectCutout();
    geomRelInf->intersects = RelInfoValue::NO;
    return false;
  }

  if (!(_config.geometryOptimizations &
        osm2rdf::config::GeometryRelationOptimization::APPROXIMATE_POLYGONS) ||
      boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    // we definitely don't intersect if ...
    if (!boost::geometry::intersects(geomA, envelopesB[0])) {
      // ... does not intersect with envelope
      geomRelInf->intersects = RelInfoValue::NO;
      stats->skippedByBox();
      return false;
    }

    if (!boost::geometry::intersects(geomA, areaOBB)) {
      // ... does not intersect with oriented bounding box
      geomRelInf->intersects = RelInfoValue::NO;
      stats->skippedByOrientedBox();
      return false;
    }

    stats->fullCheck();
    if (boost::geometry::intersects(geomA, geomB)) {
      geomRelInf->intersects = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->intersects = RelInfoValue::NO;
      return false;
    }
  }

  if (false) {
    stats->skippedByOuter();
    if (boost::geometry::intersects(geomA, outerGeomB)) {
      geomRelInf->intersects = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->intersects = RelInfoValue::NO;
      return false;
    }
  }

  bool intersects = false;
  for (size_t i = 1; i < envelopesB.size(); i++) {
    if (boost::geometry::intersects(geomA, envelopesB[i])) intersects = true;
  }

  if (!intersects) {
    // if does not intersect with envelope, we definitely don't intersect
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByBox();
    return false;
  }

  if (boost::geometry::intersects(geomA, innerGeomB)) {
    // if intersects simplified inner, we definitely intersect
    geomRelInf->intersects = RelInfoValue::YES;
    stats->skippedByInner();
    return true;
  }

  if (!boost::geometry::intersects(geomA, outerGeomB)) {
    // if NOT intersecting simplified outer, we are definitely NOT
    // intersecting
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOuter();
    return false;
  }

  if (!boost::geometry::intersects(geomA, areaOBB)) {
    // ... does not intersect with oriented bounding box
    geomRelInf->intersects = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  stats->fullCheck();
  if (boost::geometry::intersects(geomA, geomB)) {
    geomRelInf->intersects = RelInfoValue::YES;
    return true;
  }

  geomRelInf->intersects = RelInfoValue::NO;
  return false;
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::wayInArea(const SpatialWayValue& a,
                                   const SpatialAreaValue& b,
                                   GeomRelationInfo* geomRelInf,
                                   GeomRelationStats* stats) const {
  // shortcut
  if (geomRelInf->contained == RelInfoValue::YES) {
    return true;
  }

  const auto& geomA = std::get<2>(a);
  const auto& envelopeA = std::get<0>(a);
  const auto& wayBoxIds = std::get<5>(a);
  const auto& obbA = std::get<7>(b);

  const auto& geomB = std::get<2>(b);
  const auto& innerGeomB = std::get<6>(b);
  const auto& outerGeomB = std::get<7>(b);
  const auto& envelopesB = std::get<0>(b);
  const auto& areaBoxIds = std::get<8>(b);
  const auto& areaCutouts = std::get<9>(b);
  // const auto& areaConvexHull = std::get<10>(b);
  const auto& obbB = std::get<11>(b);

  if (geomRelInf->intersects == RelInfoValue::NO) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByNonIntersect();
    return false;
  }

  bool covered = false;
  for (size_t i = 1; i < envelopesB.size(); i++) {
    if (boost::geometry::covered_by(envelopeA, envelopesB[i])) {
      covered = true;
      break;
    }
  }

  if (!covered) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByBox();
    return false;
  }

  if (_config.geometryOptimizations &
      osm2rdf::config::GeometryRelationOptimization::
          OBJECT_ORIENTED_BOUNDING_BOX) {
    if (!boost::geometry::covered_by(geomA, obbB)) {
      // if geom is not covered by oriented bounding box, we are
      // not contained
      geomRelInf->contained = RelInfoValue::NO;
      stats->skippedByOrientedBox();
      return false;
    }

    if (boost::geometry::covered_by(obbA, geomB)) {
      // if geom A's OBB is covered by geom B, we are
      // definitely contained
      geomRelInf->contained = RelInfoValue::YES;
      stats->skippedByOrientedBox();
      return true;
    }
  }

  if (geomRelInf->fullContained < 0)
    boxIdIsect(wayBoxIds, areaBoxIds, geomRelInf);

  if (geomRelInf->fullContained == wayBoxIds[0].first) {
    geomRelInf->contained = RelInfoValue::YES;
    stats->skippedByBoxIdIntersect();
    return true;
  }

  // the combined number of potential contains and sure contains is not equal
  // to the number of A's boxes, so we cannot be contained
  if ((static_cast<int32_t>(geomRelInf->toCheck.size()) +
       geomRelInf->fullContained) != wayBoxIds[0].first) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByBoxIdIntersect();
    return false;
  }

  // if the way is only in one box, and area cutouts are available, we can
  // check against the corresponding cutout
  if (wayBoxIds[0].first == 1 && !areaCutouts.empty()) {
    const auto& cutout = areaCutouts.find(abs(wayBoxIds[1].first));

    if (cutout != areaCutouts.end()) {
      stats->skippedByBoxIdIntersectCutout();
      if (boost::geometry::covered_by(geomA, cutout->second)) {
        geomRelInf->contained = RelInfoValue::YES;
        return true;
      } else {
        geomRelInf->contained = RelInfoValue::NO;
        return false;
      }
    }
  }

  if (!(_config.geometryOptimizations &
        osm2rdf::config::GeometryRelationOptimization::APPROXIMATE_POLYGONS) ||
      boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB)) {
    stats->fullCheck();

    if (boost::geometry::covered_by(geomA, geomB)) {
      geomRelInf->contained = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->contained = RelInfoValue::NO;
      return false;
    }
  }

  if (false) {
    stats->skippedByOuter();
    if (boost::geometry::covered_by(geomA, outerGeomB)) {
      geomRelInf->contained = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->contained = RelInfoValue::NO;
      return false;
    }
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
    geomRelInf->contained = RelInfoValue::YES;
    stats->skippedByBox();
    return true;
  }

  if (boost::geometry::covered_by(geomA, innerGeomB)) {
    // if covered by simplified inner, we are definitely contained
    geomRelInf->contained = RelInfoValue::YES;
    stats->skippedByInner();
    return true;
  }

  if (!boost::geometry::covered_by(geomA, outerGeomB)) {
    // if NOT covered by simplified outer, we are definitely NOT
    // contained
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByOuter();
    return false;
  }

  stats->fullCheck();
  if (boost::geometry::covered_by(geomA, geomB)) {
    geomRelInf->contained = RelInfoValue::YES;
    return true;
  }

  geomRelInf->contained = RelInfoValue::NO;
  return false;
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::areaInAreaApprox(const SpatialAreaValue& a,
                                          const SpatialAreaValue& b,
                                          GeomRelationInfo* geomRelInf,
                                          GeomRelationStats* stats) const {
  const auto& entryArea = std::get<4>(a);
  const auto& entryBoxIds = std::get<8>(a);
  const auto& areaArea = std::get<4>(b);
  const auto& areaBoxIds = std::get<8>(b);
  const auto& entryEnvelopes = std::get<0>(a);
  const auto& entryConvexHull = std::get<10>(a);
  const auto& areaEnvelopes = std::get<0>(b);
  const auto& areaCutouts = std::get<9>(b);
  const auto& entryGeom = std::get<2>(a);
  const auto& areaGeom = std::get<2>(b);
  const auto& areaConvexHull = std::get<10>(b);

  if (areaArea / entryArea <= 0.95) {
    stats->skippedByAreaSize();
    return false;
  }

  bool intersects = false;
  for (size_t i = 1; i < areaEnvelopes.size(); i++) {
    for (size_t j = 1; j < entryEnvelopes.size(); j++) {
      if (boost::geometry::intersects(areaEnvelopes[i], entryEnvelopes[j]))
        intersects = true;
    }
  }

  if (!intersects) {
    stats->skippedByBox();
    return false;
  }

  if (geomRelInf->fullContained < 0)
    boxIdIsect(entryBoxIds, areaBoxIds, geomRelInf);

  if (geomRelInf->fullContained == 0 && geomRelInf->toCheck.empty()) {
    stats->skippedByBoxIdIntersect();
    return false;
  } else if (geomRelInf->fullContained == entryBoxIds[0].first) {
    stats->skippedByBoxIdIntersect();
    geomRelInf->intersectArea = entryArea;
    return true;
  } else if (entryBoxIds.front().first == 1 &&
             areaCutouts.find(abs(entryBoxIds[1].first)) != areaCutouts.end()) {
    const auto& cutout = areaCutouts.find(abs(entryBoxIds[1].first));
    osm2rdf::geometry::Area intersect;
    boost::geometry::intersection(entryGeom, cutout->second, intersect);

    geomRelInf->intersectArea = boost::geometry::area(intersect);
    stats->skippedByBoxIdIntersectCutout();
    return fabs(1 - entryArea / geomRelInf->intersectArea) < 0.05;
  } else {
    if (!boost::geometry::is_empty(entryConvexHull) &&
        !boost::geometry::is_empty(areaConvexHull) &&
        !boost::geometry::intersects(entryConvexHull, areaConvexHull)) {
      stats->skippedByConvexHull();
      return false;
    } else if (!boost::geometry::is_empty(areaConvexHull) &&
               !boost::geometry::intersects(entryGeom, areaConvexHull)) {
      stats->skippedByConvexHull();
      return false;
    } else if (!boost::geometry::is_empty(entryConvexHull) &&
               !boost::geometry::intersects(entryConvexHull, areaGeom)) {
      stats->skippedByConvexHull();
      return false;
    }

    if (!boost::geometry::is_empty(entryConvexHull) &&
        boost::geometry::covered_by(entryConvexHull, areaGeom)) {
      stats->skippedByConvexHull();
      geomRelInf->intersectArea = entryArea;
      return true;
    }

    osm2rdf::geometry::Area intersect;
    boost::geometry::intersection(entryGeom, areaGeom, intersect);

    geomRelInf->intersectArea = boost::geometry::area(intersect);
    stats->fullCheck();
    return fabs(1 - entryArea / geomRelInf->intersectArea) < 0.05;
  }
}

// ____________________________________________________________________________
template <typename W>
bool GeometryHandler<W>::areaInArea(const SpatialAreaValue& a,
                                    const SpatialAreaValue& b,
                                    GeomRelationInfo* geomRelInf,
                                    GeomRelationStats* stats) const {
  // if we don't intersect, we are not contained
  if (geomRelInf->intersects == RelInfoValue::NO) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByNonIntersect();
    return false;
  }

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
  const auto& cutoutsB = std::get<9>(b);

  const auto& obbB = std::get<11>(b);

  // if A is bigger than B, B cannot contain A
  if (areaA > areaB) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByAreaSize();
    return false;
  }

  // if A's envelope doesn't cover B's envelope, B cannot contain A
  if (!boost::geometry::covered_by(envelopesA[0], envelopesB[0])) {
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByBox();
    return false;
  }

  if (_config.geometryOptimizations &
      osm2rdf::config::GeometryRelationOptimization::INTERSECTION_CELL_IDS) {
    // if no box id intersection has been written, do it now
    if (geomRelInf->fullContained < 0) boxIdIsect(boxIdsA, boxIdsB, geomRelInf);

    // if all of A's boxes are fully contained, we are contained
    if (geomRelInf->fullContained == boxIdsA[0].first) {
      geomRelInf->contained = RelInfoValue::YES;
      stats->skippedByBoxIdIntersect();
      return true;
    }

    // else, if the number of surely contained and potentially contained boxes
    // is unequal the number of A's boxes, we are surely not contained
    if ((static_cast<int32_t>(geomRelInf->toCheck.size()) +
         geomRelInf->fullContained) != boxIdsA[0].first) {
      geomRelInf->contained = RelInfoValue::NO;
      stats->skippedByBoxIdIntersect();
      return false;
    }

    if (_config.geometryOptimizations &
        osm2rdf::config::GeometryRelationOptimization::AREA_CUTOUTS) {
      // if A is in only one box, we can check it against the corresponding
      // cutout of B, if available
      if (boxIdsA[0].first == 1 && !cutoutsB.empty()) {
        const auto& cutout = cutoutsB.find(abs(boxIdsA[1].first));

        if (cutout != cutoutsB.end()) {
          if (boost::geometry::covered_by(geomA, cutout->second)) {
            geomRelInf->contained = RelInfoValue::YES;
            stats->skippedByBoxIdIntersectCutout();
            return true;
          } else {
            stats->skippedByBoxIdIntersectCutout();
            geomRelInf->contained = RelInfoValue::NO;
            return false;
          }
        }
      }
    }
  }

  if (!(_config.geometryOptimizations &
        osm2rdf::config::GeometryRelationOptimization::APPROXIMATE_POLYGONS) ||
      boost::geometry::is_empty(innerGeomB) ||
      boost::geometry::is_empty(outerGeomB) ||
      boost::geometry::is_empty(outerGeomA) ||
      boost::geometry::is_empty(innerGeomA)) {
    stats->fullCheck();

    if (boost::geometry::covered_by(geomA, geomB)) {
      geomRelInf->contained = RelInfoValue::YES;
      return true;
    } else {
      geomRelInf->contained = RelInfoValue::NO;
      return false;
    }
  }

  if (!boost::geometry::covered_by(innerGeomA, outerGeomB)) {
    // if simplified inner is not covered by simplified outer, we are
    // definitely not contained
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByOuter();
    return false;
  }

  if (boost::geometry::covered_by(outerGeomA, innerGeomB)) {
    // if simplified outer is covered by simplified inner, we are
    // definitely contained
    geomRelInf->contained = RelInfoValue::YES;
    stats->skippedByInner();
    return true;
  }

  if (!boost::geometry::covered_by(geomA, obbB)) {
    // if geom is not covered by oriented bounding box, we are
    // not contained
    geomRelInf->contained = RelInfoValue::NO;
    stats->skippedByOrientedBox();
    return false;
  }

  stats->fullCheck();

  if (boost::geometry::covered_by(geomA, geomB)) {
    geomRelInf->contained = RelInfoValue::YES;
    return true;
  }

  geomRelInf->contained = RelInfoValue::NO;
  return false;
}

// ____________________________________________________________________________
template <typename W>
int GeometryHandler<W>::polygonOrientation(
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
BoxIdList GeometryHandler<W>::getBoxIds(
    const osm2rdf::geometry::Way& way,
    const osm2rdf::geometry::Box& envelope) const {
  int32_t startX =
      std::floor((envelope.min_corner().get<0>() + 180.0) / GRID_W);
  int32_t startY = std::floor((envelope.min_corner().get<1>() + 90.0) / GRID_H);

  int32_t endX =
      std::floor((envelope.max_corner().get<0>() + 180.0) / GRID_W) + 1;
  int32_t endY =
      std::floor((envelope.max_corner().get<1>() + 90.0) / GRID_H) + 1;

  BoxIdList boxIds;

  for (int32_t y = startY; y < endY; y++) {
    for (int32_t x = startX; x < endX; x++) {
      osm2rdf::geometry::Box box;
      box.min_corner().set<0>(x * GRID_W - 180.0);
      box.min_corner().set<1>(y * GRID_H - 90.0);
      box.max_corner().set<0>((x + 1) * GRID_W - 180.0);
      box.max_corner().set<1>((y + 1) * GRID_H - 90.0);

      if (boost::geometry::intersects(way, box)) {
        int32_t newId = y * NUM_GRID_CELLS + x + 1;
        if (!boxIds.empty() && boxIds.back().second < 254 &&
            boxIds.back().first + boxIds.back().second == newId - 1) {
          boxIds.back().second++;
        } else {
          boxIds.push_back({newId, 0});
        }
      }
    }
  }

  return boxIds;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::getBoxIds(
    const osm2rdf::geometry::Area& area, const osm2rdf::geometry::Area& inner,
    const osm2rdf::geometry::Area& outer,
    const std::vector<osm2rdf::geometry::Box>& envelopes, int xFrom, int xTo,
    int yFrom, int yTo, int xWidth, int yHeight, BoxIdList* ret,
    const osm2rdf::geometry::Area& curIsect,
    std::unordered_map<int32_t, osm2rdf::geometry::Area>* cutouts) const {
  for (int32_t y = yFrom; y < yTo; y += yHeight) {
    for (int32_t x = xFrom; x < xTo; x += xWidth) {
      int localXWidth = std::min(xTo - x, xWidth);
      int localYHeight = std::min(yTo - y, yHeight);

      osm2rdf::geometry::Box box;
      box.min_corner().set<0>(x * GRID_W - 180.0);
      box.min_corner().set<1>(y * GRID_H - 90.0);
      box.max_corner().set<0>((x + localXWidth + 1) * GRID_W - 180.0);
      box.max_corner().set<1>((y + localYHeight + 1) * GRID_H - 90.0);

      bool boxIntersects = false;
      for (size_t i = 1; i < envelopes.size(); i++) {
        if (boost::geometry::intersects(envelopes[i], box)) {
          boxIntersects = true;
        }
      }

      if (!boxIntersects) {
        continue;
      }

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
          // compute cutout if requested
          osm2rdf::geometry::Area intersection;
          if (cutouts) {
            boost::geometry::intersection(bboxPoly, curIsect, intersection);
          }

          if (localXWidth == 1 && localYHeight == 1) {
            // only intersecting
            int32_t newId = -(y * NUM_GRID_CELLS + x + 1);

            if (cutouts) {
              (*cutouts)[-newId] = intersection;
            }

            if (!ret->empty() && ret->back().second < 254 &&
                ret->back().first - ret->back().second == newId + 1) {
              ret->back().second++;
            } else {
              ret->push_back({newId, 0});
            }
          } else {
            // we need to check in detail on a smaller level!
            // recurse down...
            int newXWidth = (localXWidth + 1) / 2;
            int newYHeight = (localYHeight + 1) / 2;

            getBoxIds(area, inner, outer, envelopes, x, x + localXWidth, y,
                      y + localYHeight, newXWidth, newYHeight, ret,
                      intersection, cutouts);
          }
        }
      }
    }
  }
}

// ____________________________________________________________________________
template <typename W>
BoxIdList GeometryHandler<W>::getBoxIds(
    const osm2rdf::geometry::Area& area,
    const std::vector<osm2rdf::geometry::Box>& envelopes,
    const osm2rdf::geometry::Area& inner, const osm2rdf::geometry::Area& outer,
    std::unordered_map<int32_t, osm2rdf::geometry::Area>* cutouts) const {
  size_t totNumPoints = 0;
  for (const auto& p : area) totNumPoints += p.outer().size();

  int32_t startX =
      std::floor((envelopes[0].min_corner().get<0>() + 180.0) / GRID_W);
  int32_t startY =
      std::floor((envelopes[0].min_corner().get<1>() + 90.0) / GRID_H);

  int32_t endX =
      std::floor((envelopes[0].max_corner().get<0>() + 180.0) / GRID_W) + 1;
  int32_t endY =
      std::floor((envelopes[0].max_corner().get<1>() + 90.0) / GRID_H) + 1;

  BoxIdList boxIds;

  getBoxIds(area, inner, outer, envelopes, startX, endX, startY, endY,
            (endX - startX + 3) / 4, (endY - startY + 3) / 4, &boxIds, area,
            cutouts);
  std::sort(boxIds.begin(), boxIds.end(), BoxIdCmp());

  return boxIds;
}

// ____________________________________________________________________________
template <typename W>
int32_t GeometryHandler<W>::getBoxId(
    const osm2rdf::geometry::Location& p) const {
  int32_t x = floor((p.get<0>() + 180.0) / GRID_W);
  int32_t y = floor((p.get<1>() + 90.0) / GRID_H);

  return y * NUM_GRID_CELLS + x + 1;
}

// ____________________________________________________________________________
template <typename W>
std::string GeometryHandler<W>::areaNS(AreaFromType type) const {
  switch (type) {
    case AreaFromType::RELATION:
      return osm2rdf::ttl::constants::NAMESPACE__OSM_RELATION;
    case AreaFromType::WAY:
      return osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
    default:
      return osm2rdf::ttl::constants::NAMESPACE__OSM_WAY;
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::boxIdIsect(const BoxIdList& idsA,
                                    const BoxIdList& idsB,
                                    GeomRelationInfo* geomRelInf) const {
  geomRelInf->fullContained = 0;

  // shortcuts
  if (abs(idsA[1].first) > abs(idsB.back().first) + idsB.back().second) {
    return;
  }
  if (abs(idsA.back().first) + idsA.back().second < idsB[1].first) {
    return;
  }

  size_t i = 1;
  int32_t ii = 0;
  size_t j = 1;
  int32_t jj = 0;

  bool noContained = false;

  while (i < idsA.size() && j < idsB.size()) {
    if (abs(idsA[i].first) + ii == abs(idsB[j].first) + jj) {
      if (idsB[j].first > 0) {
        geomRelInf->fullContained++;

        // we now know that we surely intersect. If we know already that
        // we cannot be contained, return here
        if (noContained) {
          return;
        }
      }
      if (idsB[j].first < 0) {
        geomRelInf->toCheck.push_back(abs(idsA[i].first));
      }

      if (++ii > idsA[i].second) {
        i++;
        ii = 0;
      }
      if (++jj > idsB[j].second) {
        j++;
        jj = 0;
      }
    } else if (abs(idsA[i].first) + ii < abs(idsB[j].first) + jj) {
      // if we already know that we intersect, we are now sure that we
      // cannot be contained - it is irrelevant by how "much" we cannot be
      // contained, so just return
      if (geomRelInf->fullContained > 0) {
        return;
      }

      // set noContained marker to true for later
      noContained = true;

      if (abs(idsA[i].first) + idsA[i].second < abs(idsB[j].first) + jj) {
        // entire block smaller, jump it
        ii = 0;
        i++;
      } else {
        if (++ii > idsA[i].second) {
          i++;
          ii = 0;
        }
      }
    } else {
      size_t gallop = 1;
      do {
        auto end = idsB.end();
        if (j + gallop < idsB.size()) {
          end = idsB.begin() + j + gallop;
        }

        if (end == idsB.end() || abs(end->first) >= abs(idsA[i].first) + ii) {
          jj = 0;
          j = std::lower_bound(idsB.begin() + j + gallop / 2, end,
                               abs(idsA[i].first) + ii, BoxIdCmp()) -
              idsB.begin();
          if (j > 0 && abs(idsB[j - 1].first) < abs(idsA[i].first) + ii &&
              abs(idsB[j - 1].first) + idsB[j - 1].second >=
                  abs(idsA[i].first) + ii) {
            j--;
            jj = (abs(idsA[i].first) + ii) - abs(idsB[j].first);
          }
          break;
        }

        gallop *= 2;

      } while (true);
    }
  }
}

// ____________________________________________________________________________
template <typename W>
BoxIdList GeometryHandler<W>::pack(const BoxIdList& ids) const {
  if (ids.empty()) {
    return {{0, 0}};
  }
  // assume the list is sorted!

  BoxIdList ret;
  // dummy value, will later hold number of entries
  ret.push_back({ids.front().second + 1, 0});
  ret.push_back(ids.front());

  for (size_t i = 1; i < ids.size(); i++) {
    ret[0].first += ids[i].second + 1;
    if (ret.back().second < 254 - ids[i].second && ids[i].first > 0 &&
        ret.back().first > 0 &&
        ret.back().first + ret.back().second == ids[i].first - 1) {
      ret.back().second += 1 + ids[i].second;
    } else if (ret.back().second < 254 - ids[i].second && ids[i].first < 0 &&
               ret.back().first < 0 &&
               ret.back().first - ret.back().second == ids[i].first + 1) {
      ret.back().second += 1 + ids[i].second;
    } else {
      ret.push_back(ids[i]);
    }
  }

  return ret;
}

// ____________________________________________________________________________
template <typename W>
std::vector<SpatialAreaRefValue> GeometryHandler<W>::indexQryCover(
    const SpatialAreaValue& area) const {
  std::vector<SpatialAreaRefValue> queryResult;

  const auto& envelopes = std::get<0>(area);

  for (size_t i = 1; i < envelopes.size(); i++) {
    _spatialIndex.query(boost::geometry::index::covers(envelopes[i]),
                        std::back_inserter(queryResult));
  }

  unique(queryResult);

  return queryResult;
}

// ____________________________________________________________________________
template <typename W>
std::vector<SpatialAreaRefValue> GeometryHandler<W>::indexQryIntersect(
    const SpatialAreaValue& area) const {
  std::vector<SpatialAreaRefValue> queryResult;

  const auto& envelopes = std::get<0>(area);

  for (size_t i = 1; i < envelopes.size(); i++) {
    _spatialIndex.query(boost::geometry::index::intersects(envelopes[i]),
                        std::back_inserter(queryResult));
  }

  unique(queryResult);

  return queryResult;
}

// ____________________________________________________________________________
template <typename W>
std::vector<SpatialAreaRefValue> GeometryHandler<W>::indexQry(
    const SpatialNodeValue& node) const {
  std::vector<SpatialAreaRefValue> queryResult;

  osm2rdf::geometry::Box nodeEnvelope;
  boost::geometry::envelope(std::get<1>(node), nodeEnvelope);

  _spatialIndex.query(boost::geometry::index::covers(nodeEnvelope),
                      std::back_inserter(queryResult));

  unique(queryResult);

  return queryResult;
}

// ____________________________________________________________________________
template <typename W>
std::vector<SpatialAreaRefValue> GeometryHandler<W>::indexQryIntersect(
    const SpatialWayValue& way) const {
  std::vector<SpatialAreaRefValue> queryResult;

  const auto& envelopes = std::get<4>(way);

  for (size_t i = 0; i < envelopes.size(); i++) {
    _spatialIndex.query(boost::geometry::index::intersects(envelopes[i]),
                        std::back_inserter(queryResult));
  }

  unique(queryResult);

  return queryResult;
}

// ____________________________________________________________________________
template <typename W>
std::vector<SpatialAreaRefValue> GeometryHandler<W>::wayIndexQryIntersect(
    const SpatialWayValue& way) const {
  std::vector<SpatialAreaRefValue> queryResult;

  const auto& envelopes = std::get<4>(way);

  for (size_t i = 0; i < envelopes.size(); i++) {
    _spatialWayIndex.query(boost::geometry::index::intersects(envelopes[i]),
                        std::back_inserter(queryResult));
  }

  unique(queryResult);

  return queryResult;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::unique(std::vector<SpatialAreaRefValue>& refs) const {
  // remove duplicates, may occur since we used multiple envelope queries
  // to build the result!
  std::sort(refs.begin(), refs.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
  auto last = std::unique(
      refs.begin(), refs.end(),
      [](const auto& a, const auto& b) { return a.second == b.second; });

  // duplicates were swapped to the end of vector, beginning at last
  refs.erase(last, refs.end());

  // small -> big
  std::sort(refs.begin(), refs.end(), [this](const auto& a, const auto& b) {
    return std::get<4>(_spatialStorageArea[a.second]) <
           std::get<4>(_spatialStorageArea[b.second]);
  });
}

// ____________________________________________________________________________
template <typename W>
uint8_t GeometryHandler<W>::borderContained(Way::id_t wayId,
                                            Area::id_t areaId) const {
  const auto& relations = _areaBorderWaysIndex.find(wayId);
  if (relations != _areaBorderWaysIndex.end()) {
    auto a = std::lower_bound(relations->second.begin(),
                              relations->second.end(), areaId, MemberRelCmp());

    if (a != relations->second.end() && a->first == areaId) {
      // if inner, return 1
      if (a->second) {
        return 1;
      }
      // else if outer, return 2
      return 2;
    }
  }

  return 0;
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::writeTransitiveClosure(
    const std::vector<osm2rdf::osm::Area::id_t>& successors,
    const std::string& entryIRI, const std::string& rel,
    const std::string& symmRel) {
  // transitive closure
  if (_config.writeGeomRelTransClosure) {
    for (const auto& succ : successors) {
      auto succIdx = _spatialStorageAreaIndex[succ];
      const auto& succAreaId = std::get<3>(_spatialStorageArea[succIdx]);
      const auto& succAreaFromType = std::get<5>(_spatialStorageArea[succIdx]);
      const auto& succAreaIRI =
          _writer->generateIRI(areaNS(succAreaFromType), succAreaId);

      _writer->writeTriple(succAreaIRI, rel, entryIRI);
      _writer->writeTriple(entryIRI, symmRel, succAreaIRI);
    }
  }
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::writeTransitiveClosure(
    const std::vector<osm2rdf::osm::Area::id_t>& successors,
    const std::string& entryIRI, const std::string& rel) {
  // transitive closure
  if (_config.writeGeomRelTransClosure) {
    for (const auto& succ : successors) {
      auto succIdx = _spatialStorageAreaIndex[succ];
      const auto& succAreaId = std::get<3>(_spatialStorageArea[succIdx]);
      const auto& succAreaFromType = std::get<5>(_spatialStorageArea[succIdx]);
      const auto& succAreaIRI =
          _writer->generateIRI(areaNS(succAreaFromType), succAreaId);

      _writer->writeTriple(succAreaIRI, rel, entryIRI);
    }
  }
}

// ____________________________________________________________________________
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER>;

#pragma clang diagnostic pop
