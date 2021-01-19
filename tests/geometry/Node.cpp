// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Node.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Node getDefaultObject() {
  return osm2ttl::geometry::Node();
}

osm2ttl::geometry::Node getFilledObject() {
  return osm2ttl::geometry::Node(10, 20);
}

TEST(Node, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Node origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Node origFilledObject = getFilledObject();
  osm2ttl::geometry::Node loadedDefaultObject;
  osm2ttl::geometry::Node loadedFilledObject;

  // Store and load
  boost::archive::binary_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  //std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.x(), loadedDefaultObject.x());
  ASSERT_EQ(origDefaultObject.y(), loadedDefaultObject.y());
  ASSERT_EQ(origFilledObject.x(), loadedFilledObject.x());
  ASSERT_EQ(origFilledObject.y(), loadedFilledObject.y());
}

TEST(Node, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Node origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Node origFilledObject = getFilledObject();
  osm2ttl::geometry::Node loadedDefaultObject;
  osm2ttl::geometry::Node loadedFilledObject;

  // Store and load
  boost::archive::text_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  //std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.x(), loadedDefaultObject.x());
  ASSERT_EQ(origDefaultObject.y(), loadedDefaultObject.y());
  ASSERT_EQ(origFilledObject.x(), loadedFilledObject.x());
  ASSERT_EQ(origFilledObject.y(), loadedFilledObject.y());
}

}