// Copyright 2023, University of Freiburg
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

#ifndef OSM2RDF_OSM_GENERIC_H
#define OSM2RDF_OSM_GENERIC_H

#include "boost/geometry/geometry.hpp"
#include "osm2rdf/geometry/Box.h"
#include "osm2rdf/geometry/Node.h"
#include "osm2rdf/geometry/Polygon.h"

namespace osm2rdf::osm::generic {

// ____________________________________________________________________________
inline osm2rdf::geometry::Polygon boxToPolygon(
    const osm2rdf::geometry::Box& box) {
  // Explicitly convert box to closed polygon -> 5 coordinates.
  return osm2rdf::geometry::Polygon{
      {{box.min_corner().x(), box.min_corner().y()},
       {box.min_corner().x(), box.max_corner().y()},
       {box.max_corner().x(), box.max_corner().y()},
       {box.max_corner().x(), box.min_corner().y()},
       {box.min_corner().x(), box.min_corner().y()}}};
};

// ____________________________________________________________________________
inline osm2rdf::geometry::Node rotateNodeByAngle(
    const osm2rdf::geometry::Node& point, double angle) {
  return osm2rdf::geometry::Node{
      point.x() * std::cos(angle) - point.y() * std::sin(angle),
      point.x() * std::sin(angle) + point.y() * std::cos(angle)};
};

// ____________________________________________________________________________
inline osm2rdf::geometry::Polygon orientedBoundingBoxFromConvexHull(
    const osm2rdf::geometry::Polygon& convexHull) {
  if (convexHull.outer().size() < 2) {
    return convexHull;
  }

  // Store angle and box coordinates of minimum bounding box
  double minimalBoxAngle = 0;
  double minX = std::numeric_limits<double>::infinity();
  double maxX = -std::numeric_limits<double>::infinity();
  double minY = std::numeric_limits<double>::infinity();
  double maxY = -std::numeric_limits<double>::infinity();
  osm2rdf::geometry::Box minimalBox{{minX, minY}, {maxX, maxY}};

  // for each segment ...
  for (size_t i = 0; i < convexHull.outer().size(); ++i) {
    // ... determine points ...
    osm2rdf::geometry::Node pointA = convexHull.outer().at(i);
    osm2rdf::geometry::Node pointB =
        convexHull.outer().at((i + 1) % convexHull.outer().size());

    // ... and the angle of current segment to x axis ...
    double angle =
        -std::atan2(pointA.y() - pointB.y(), pointA.x() - pointB.x());

    // ... rotate each node in the hull to find new min and max values ...
    for (size_t j = 0; j < convexHull.outer().size(); ++j) {
      auto rotatedNode = rotateNodeByAngle(convexHull.outer().at(j), angle);
      minX = std::min(minX, rotatedNode.x());
      maxX = std::max(maxX, rotatedNode.x());
      minY = std::min(minY, rotatedNode.y());
      maxY = std::max(maxY, rotatedNode.y());
    }
    // ... create new box and determine if smaller than previous box.
    osm2rdf::geometry::Box box{{minX, minY}, {maxX, maxY}};
    if (boost::geometry::area(minimalBox) > boost::geometry::area(box)) {
      minimalBox = box;
      minimalBoxAngle = angle;
    }
  }
  // convert box to polygon ...
  osm2rdf::geometry::Polygon tmpObb = boxToPolygon(minimalBox);
  // ... rotate the polygon by the negative angle ...
  return osm2rdf::geometry::Polygon{
      {{rotateNodeByAngle(tmpObb.outer().at(0), -minimalBoxAngle)},
       {rotateNodeByAngle(tmpObb.outer().at(1), -minimalBoxAngle)},
       {rotateNodeByAngle(tmpObb.outer().at(2), -minimalBoxAngle)},
       {rotateNodeByAngle(tmpObb.outer().at(3), -minimalBoxAngle)},
       {rotateNodeByAngle(tmpObb.outer().at(4), -minimalBoxAngle)}}};
};

}  // namespace osm2rdf::osm::generic

#endif  // OSM2RDF_OSM_GENERIC_H
