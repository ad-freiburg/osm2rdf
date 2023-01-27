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

#include "osm2rdf/osm/Generic.h"

#include "gtest/gtest.h"

const double TRIGONOMETRY_EPSILON = 0.00001;

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_Generic, boxToPolygon) {
  osm2rdf::geometry::Polygon p =
      osm2rdf::osm::generic::boxToPolygon({{0, 0}, {1, 1}});
  // 5 Points
  ASSERT_EQ(5, p.outer().size());
  // Last point equals first point
  ASSERT_DOUBLE_EQ(p.outer().at(0).x(), p.outer().at(4).x());
  ASSERT_DOUBLE_EQ(p.outer().at(0).y(), p.outer().at(4).y());
  // bottom left -> top left -> top right -> bottom right
  ASSERT_DOUBLE_EQ(p.outer().at(0).x(), p.outer().at(1).x());
  ASSERT_DOUBLE_EQ(p.outer().at(1).y(), p.outer().at(2).y());
  ASSERT_DOUBLE_EQ(p.outer().at(2).x(), p.outer().at(3).x());
  ASSERT_DOUBLE_EQ(p.outer().at(3).y(), p.outer().at(4).y());
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngleOriginPoint) {
  for (double alpha = 0; alpha < 2 * 3.141592; alpha += 0.001) {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 0}, 0);
    ASSERT_EQ(p.x(), 0);
    ASSERT_EQ(p.y(), 0);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngle0Degree) {
  double angle = 0;
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_EQ(p.x(), 1);
    ASSERT_EQ(p.y(), 0);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_EQ(p.x(), 1);
    ASSERT_EQ(p.y(), 1);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_EQ(p.x(), 0);
    ASSERT_EQ(p.y(), 1);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngle90DegreeRotation) {
  double angle = 3.141592 / 2;
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_NEAR(p.x(), 0, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 0, TRIGONOMETRY_EPSILON);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngle180DegreeRotation) {
  double angle = 3.141592;
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 0, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_NEAR(p.x(), 0, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngle270DegreeRotation) {
  double angle = 3 * (3.141592 / 2);
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_NEAR(p.x(), 0, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_NEAR(p.x(), 1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_NEAR(p.x(), 1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 0, TRIGONOMETRY_EPSILON);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngleMinus0Degree) {
  double angle = -0;
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_EQ(p.x(), 1);
    ASSERT_EQ(p.y(), 0);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_EQ(p.x(), 1);
    ASSERT_EQ(p.y(), 1);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_EQ(p.x(), 0);
    ASSERT_EQ(p.y(), 1);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngleMinus90DegreeRotation) {
  double angle = -3.141592 / 2;
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_NEAR(p.x(), 0, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_NEAR(p.x(), 1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_NEAR(p.x(), 1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 0, TRIGONOMETRY_EPSILON);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngleMinus180DegreeRotation) {
  double angle = -3.141592;
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 0, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_NEAR(p.x(), 0, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), -1, TRIGONOMETRY_EPSILON);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, rotateNodeByAngleMinus270DegreeRotation) {
  double angle = -3 * (3.141592 / 2);
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 0}, angle);
    ASSERT_NEAR(p.x(), 0, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({1, 1}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 1, TRIGONOMETRY_EPSILON);
  }
  {
    osm2rdf::geometry::Node p =
        osm2rdf::osm::generic::rotateNodeByAngle({0, 1}, angle);
    ASSERT_NEAR(p.x(), -1, TRIGONOMETRY_EPSILON);
    ASSERT_NEAR(p.y(), 0, TRIGONOMETRY_EPSILON);
  }
}

// ____________________________________________________________________________
TEST(OSM_Generic, orientedBoundingBoxFromConvexHullAxisAligned) {
  osm2rdf::geometry::Polygon hull =
      osm2rdf::osm::generic::boxToPolygon({{0, 0}, {2, 1}});
  osm2rdf::geometry::Polygon obb =
      osm2rdf::osm::generic::orientedBoundingBoxFromConvexHull(hull);
  ASSERT_NEAR(obb.outer().at(0).x(), 0, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(0).y(), 1, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(1).x(), 2, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(1).y(), 1, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(2).x(), 2, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(2).y(), 0, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(3).x(), 0, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(3).y(), 0, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(4).x(), 0, TRIGONOMETRY_EPSILON);
  ASSERT_NEAR(obb.outer().at(4).y(), 1, TRIGONOMETRY_EPSILON);
}

}  // namespace osm2rdf::osm
