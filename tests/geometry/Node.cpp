// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Node.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Node getDefaultObject() { return osm2ttl::geometry::Node(); }

osm2ttl::geometry::Node getFilledObject() {
  return osm2ttl::geometry::Node(10, 20);
}

TEST(GEOMETRY_Node, equalsOperator) {
  osm2ttl::geometry::Node o1(10, 10);
  osm2ttl::geometry::Node o2(10, 20);
  osm2ttl::geometry::Node o3(20, 10);

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

TEST(GEOMETRY_Node, notEqualsOperator) {
  osm2ttl::geometry::Node o1(10, 10);
  osm2ttl::geometry::Node o2(10, 20);
  osm2ttl::geometry::Node o3(20, 10);

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

TEST(GEOMETRY_Node, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Node origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Node origFilledObject = getFilledObject();
  osm2ttl::geometry::Node loadedDefaultObject;
  osm2ttl::geometry::Node loadedFilledObject;

  // Store and load
  boost::archive::binary_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_TRUE(origDefaultObject == loadedDefaultObject);
  ASSERT_TRUE(origFilledObject == loadedFilledObject);
}

TEST(GEOMETRY_Node, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Node origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Node origFilledObject = getFilledObject();
  osm2ttl::geometry::Node loadedDefaultObject;
  osm2ttl::geometry::Node loadedFilledObject;

  // Store and load
  boost::archive::text_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_TRUE(origDefaultObject == loadedDefaultObject);
  ASSERT_TRUE(origFilledObject == loadedFilledObject);
}

}  // namespace osm2ttl::geometry