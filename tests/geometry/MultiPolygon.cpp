// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/MultiPolygon.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

// ____________________________________________________________________________
osm2ttl::geometry::MultiPolygon getDefaultObject() {
  return osm2ttl::geometry::MultiPolygon();
}

// ____________________________________________________________________________
osm2ttl::geometry::MultiPolygon getFilledObject() {
  osm2ttl::geometry::MultiPolygon obj;
  obj.resize(2);
  obj[0].outer().reserve(3);
  obj[0].outer().push_back(Location{0, 0});
  obj[0].outer().push_back(Location{0, 1});
  obj[0].outer().push_back(Location{1, 0});
  obj[0].inners().resize(0);
  obj[1].outer().reserve(4);
  obj[1].outer().push_back(Location{10, 10});
  obj[1].outer().push_back(Location{10, 20});
  obj[1].outer().push_back(Location{20, 20});
  obj[1].outer().push_back(Location{20, 10});
  obj[1].inners().resize(1);
  obj[1].inners()[0].reserve(3);
  obj[1].inners()[0].push_back(Location{14, 14});
  obj[1].inners()[0].push_back(Location{14, 16});
  obj[1].inners()[0].push_back(Location{16, 14});
  return obj;
}

// ____________________________________________________________________________
TEST(GEOMETRY_MultiPolygon, equalsOperator) {
  osm2ttl::geometry::MultiPolygon o1;
  o1.resize(2);
  o1[0].outer().reserve(3);
  o1[0].outer().push_back(Location{0, 0});
  o1[0].outer().push_back(Location{0, 1});
  o1[0].outer().push_back(Location{1, 0});
  o1[0].inners().resize(0);
  o1[1].outer().reserve(4);
  o1[1].outer().push_back(Location{10, 10});
  o1[1].outer().push_back(Location{10, 20});
  o1[1].outer().push_back(Location{20, 20});
  o1[1].outer().push_back(Location{20, 10});
  o1[1].inners().resize(1);
  o1[1].inners()[0].reserve(3);
  o1[1].inners()[0].push_back(Location{14, 14});
  o1[1].inners()[0].push_back(Location{14, 16});
  o1[1].inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::MultiPolygon o2;
  o2.resize(2);
  o2[0].outer().reserve(3);
  o2[0].outer().push_back(Location{0, 0});
  o2[0].outer().push_back(Location{1, 0});
  o2[0].outer().push_back(Location{0, 1});
  o2[0].inners().resize(0);
  o2[1].outer().reserve(4);
  o2[1].outer().push_back(Location{10, 10});
  o2[1].outer().push_back(Location{10, 20});
  o2[1].outer().push_back(Location{20, 20});
  o2[1].outer().push_back(Location{20, 10});
  o2[1].inners().resize(1);
  o2[1].inners()[0].reserve(3);
  o2[1].inners()[0].push_back(Location{14, 14});
  o2[1].inners()[0].push_back(Location{14, 16});
  o2[1].inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::MultiPolygon o3;
  o3.resize(2);
  o3[0].outer().reserve(3);
  o3[0].outer().push_back(Location{0, 0});
  o3[0].outer().push_back(Location{0, 1});
  o3[0].outer().push_back(Location{1, 0});
  o3[0].inners().resize(0);
  o3[1].outer().reserve(4);
  o3[1].outer().push_back(Location{10, 10});
  o3[1].outer().push_back(Location{10, 20});
  o3[1].outer().push_back(Location{20, 20});
  o3[1].outer().push_back(Location{20, 10});
  o3[1].inners().resize(1);
  o3[1].inners()[0].reserve(2);
  o3[1].inners()[0].push_back(Location{14, 14});
  o3[1].inners()[0].push_back(Location{14, 16});

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
TEST(GEOMETRY_MultiPolygon, notEqualsOperator) {
  osm2ttl::geometry::MultiPolygon o1;
  o1.resize(2);
  o1[0].outer().reserve(3);
  o1[0].outer().push_back(Location{0, 0});
  o1[0].outer().push_back(Location{0, 1});
  o1[0].outer().push_back(Location{1, 0});
  o1[0].inners().resize(0);
  o1[1].outer().reserve(4);
  o1[1].outer().push_back(Location{10, 10});
  o1[1].outer().push_back(Location{10, 20});
  o1[1].outer().push_back(Location{20, 20});
  o1[1].outer().push_back(Location{20, 10});
  o1[1].inners().resize(1);
  o1[1].inners()[0].reserve(3);
  o1[1].inners()[0].push_back(Location{14, 14});
  o1[1].inners()[0].push_back(Location{14, 16});
  o1[1].inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::MultiPolygon o2;
  o2.resize(2);
  o2[0].outer().reserve(3);
  o2[0].outer().push_back(Location{0, 0});
  o2[0].outer().push_back(Location{1, 0});
  o2[0].outer().push_back(Location{0, 1});
  o2[0].inners().resize(0);
  o2[1].outer().reserve(4);
  o2[1].outer().push_back(Location{10, 10});
  o2[1].outer().push_back(Location{10, 20});
  o2[1].outer().push_back(Location{20, 20});
  o2[1].outer().push_back(Location{20, 10});
  o2[1].inners().resize(1);
  o2[1].inners()[0].reserve(3);
  o2[1].inners()[0].push_back(Location{14, 14});
  o2[1].inners()[0].push_back(Location{14, 16});
  o2[1].inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::MultiPolygon o3;
  o3.resize(2);
  o3[0].outer().reserve(3);
  o3[0].outer().push_back(Location{0, 0});
  o3[0].outer().push_back(Location{0, 1});
  o3[0].outer().push_back(Location{1, 0});
  o3[0].inners().resize(0);
  o3[1].outer().reserve(4);
  o3[1].outer().push_back(Location{10, 10});
  o3[1].outer().push_back(Location{10, 20});
  o3[1].outer().push_back(Location{20, 20});
  o3[1].outer().push_back(Location{20, 10});
  o3[1].inners().resize(1);
  o3[1].inners()[0].reserve(2);
  o3[1].inners()[0].push_back(Location{14, 14});
  o3[1].inners()[0].push_back(Location{14, 16});

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
TEST(GEOMETRY_MultiPolygon, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::MultiPolygon origDefaultObject = getDefaultObject();
  osm2ttl::geometry::MultiPolygon origFilledObject = getFilledObject();
  osm2ttl::geometry::MultiPolygon loadedDefaultObject;
  osm2ttl::geometry::MultiPolygon loadedFilledObject;

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

// ____________________________________________________________________________
TEST(GEOMETRY_MultiPolygon, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::MultiPolygon origDefaultObject = getDefaultObject();
  osm2ttl::geometry::MultiPolygon origFilledObject = getFilledObject();
  osm2ttl::geometry::MultiPolygon loadedDefaultObject;
  osm2ttl::geometry::MultiPolygon loadedFilledObject;

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