// Copyright 2022, University of Freiburg
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

#include "osm2rdf/geometry/Relation.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

#if BOOST_VERSION >= 107800

namespace osm2rdf::geometry {

// ____________________________________________________________________________
osm2rdf::geometry::Relation getDefaultObject() {
  return osm2rdf::geometry::Relation();
}

// ____________________________________________________________________________
osm2rdf::geometry::Relation getFilledObject() {
  osm2rdf::geometry::Relation obj;
  return obj;
}

// ____________________________________________________________________________
TEST(GEOMETRY_Relation, serializationBinary) {
  std::stringstream boostBuffer;

  osm2rdf::geometry::Relation origDefaultObject = getDefaultObject();
  osm2rdf::geometry::Relation origFilledObject = getFilledObject();
  osm2rdf::geometry::Relation loadedDefaultObject;
  osm2rdf::geometry::Relation loadedFilledObject;

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
TEST(GEOMETRY_Relation, serializationText) {
  std::stringstream boostBuffer;

  osm2rdf::geometry::Relation origDefaultObject = getDefaultObject();
  osm2rdf::geometry::Relation origFilledObject = getFilledObject();
  osm2rdf::geometry::Relation loadedDefaultObject;
  osm2rdf::geometry::Relation loadedFilledObject;

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
#endif  // BOOST_VERSION >= 107800