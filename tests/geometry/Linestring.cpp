// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Linestring.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Linestring getDefaultObject() {
  return osm2ttl::geometry::Linestring();
}

osm2ttl::geometry::Linestring getFilledObject() {
  osm2ttl::geometry::Linestring obj;
  obj.push_back(Location{0, 0});
  obj.push_back(Location{5, 0});
  obj.push_back(Location{0, 5});
  obj.push_back(Location{10, 10});
  return obj;
}

TEST(Linestring, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Linestring origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Linestring origFilledObject = getFilledObject();
  osm2ttl::geometry::Linestring loadedDefaultObject;
  osm2ttl::geometry::Linestring loadedFilledObject;

  // Store and load
  boost::archive::binary_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.size(), loadedDefaultObject.size());
  ASSERT_EQ(origFilledObject.size(), loadedFilledObject.size());
  for (size_t i = 0; i < loadedFilledObject.size(); ++i) {
    ASSERT_EQ(origFilledObject[i].x(), loadedFilledObject[i].x());
    ASSERT_EQ(origFilledObject[i].y(), loadedFilledObject[i].y());
  }
}

TEST(Linestring, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Linestring origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Linestring origFilledObject = getFilledObject();
  osm2ttl::geometry::Linestring loadedDefaultObject;
  osm2ttl::geometry::Linestring loadedFilledObject;

  // Store and load
  boost::archive::text_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.size(), loadedDefaultObject.size());
  ASSERT_EQ(origFilledObject.size(), loadedFilledObject.size());
  for (size_t i = 0; i < loadedFilledObject.size(); ++i) {
    ASSERT_EQ(origFilledObject[i].x(), loadedFilledObject[i].x());
    ASSERT_EQ(origFilledObject[i].y(), loadedFilledObject[i].y());
  }
}

}  // namespace osm2ttl::geometry