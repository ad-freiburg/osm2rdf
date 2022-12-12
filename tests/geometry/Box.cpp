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

#include "osm2rdf/geometry/Box.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2rdf::geometry {

// ____________________________________________________________________________
osm2rdf::geometry::Box getDefaultObject() { return osm2rdf::geometry::Box(); }

// ____________________________________________________________________________
osm2rdf::geometry::Box getFilledObject() {
  osm2rdf::geometry::Box obj;
  obj.min_corner() = Location{50, 50};
  obj.max_corner() = Location{200, 200};
  return obj;
}

// ____________________________________________________________________________
TEST(GEOMETRY_Box, equalsOperator) {
  osm2rdf::geometry::Box o1;
  o1.min_corner() = Location{50, 50};
  o1.max_corner() = Location{200, 200};

  osm2rdf::geometry::Box o2;
  o2.min_corner() = Location{50, 200};
  o2.max_corner() = Location{200, 200};

  osm2rdf::geometry::Box o3;
  o3.min_corner() = Location{200, 200};
  o3.max_corner() = Location{50, 50};

  ASSERT_TRUE(o1 == o1);
  ASSERT_FALSE(o1 == o2);
  ASSERT_FALSE(o1 == o3);

  ASSERT_FALSE(o2 == o1);
  ASSERT_TRUE(o2 == o2);
  ASSERT_FALSE(o2 == o3);

  ASSERT_FALSE(o3 == o1);
  ASSERT_FALSE(o3 == o2);
  ASSERT_TRUE(o3 == o3);
}

// ____________________________________________________________________________
TEST(GEOMETRY_Box, notEqualsOperator) {
  osm2rdf::geometry::Box o1;
  o1.min_corner() = Location{50, 50};
  o1.max_corner() = Location{200, 200};

  osm2rdf::geometry::Box o2;
  o2.min_corner() = Location{50, 200};
  o2.max_corner() = Location{200, 200};

  osm2rdf::geometry::Box o3;
  o3.min_corner() = Location{200, 200};
  o3.max_corner() = Location{50, 50};

  ASSERT_FALSE(o1 != o1);
  ASSERT_TRUE(o1 != o2);
  ASSERT_TRUE(o1 != o3);

  ASSERT_TRUE(o2 != o1);
  ASSERT_FALSE(o2 != o2);
  ASSERT_TRUE(o2 != o3);

  ASSERT_TRUE(o3 != o1);
  ASSERT_TRUE(o3 != o2);
  ASSERT_FALSE(o3 != o3);
}

// ____________________________________________________________________________
TEST(GEOMETRY_Box, serializationBinary) {
  std::stringstream boostBuffer;

  osm2rdf::geometry::Box origDefaultObject = getDefaultObject();
  osm2rdf::geometry::Box origFilledObject = getFilledObject();
  osm2rdf::geometry::Box loadedDefaultObject;
  osm2rdf::geometry::Box loadedFilledObject;

  // Store and load
  boost::archive::binary_oarchive oa(boostBuffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << boostBuffer.str() << std::endl;
  boost::archive::binary_iarchive ia(boostBuffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_TRUE(origDefaultObject == loadedDefaultObject);
  ASSERT_TRUE(origFilledObject == loadedFilledObject);
}

// ____________________________________________________________________________
TEST(GEOMETRY_Box, serializationText) {
  std::stringstream boostBuffer;

  osm2rdf::geometry::Box origDefaultObject = getDefaultObject();
  osm2rdf::geometry::Box origFilledObject = getFilledObject();
  osm2rdf::geometry::Box loadedDefaultObject;
  osm2rdf::geometry::Box loadedFilledObject;

  // Store and load
  boost::archive::text_oarchive oa(boostBuffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << boostBuffer.str() << std::endl;
  boost::archive::text_iarchive ia(boostBuffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_TRUE(origDefaultObject == loadedDefaultObject);
  ASSERT_TRUE(origFilledObject == loadedFilledObject);
}

}  // namespace osm2rdf::geometry