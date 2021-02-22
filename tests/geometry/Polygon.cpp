// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Polygon.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Polygon getDefaultObject() {
  return osm2ttl::geometry::Polygon();
}

osm2ttl::geometry::Polygon getFilledObject() {
  osm2ttl::geometry::Polygon obj;
  obj.outer().reserve(4);
  obj.outer().push_back(Location{10, 10});
  obj.outer().push_back(Location{10, 20});
  obj.outer().push_back(Location{20, 20});
  obj.outer().push_back(Location{20, 10});
  obj.inners().resize(1);
  obj.inners()[0].reserve(3);
  obj.inners()[0].push_back(Location{14, 14});
  obj.inners()[0].push_back(Location{14, 16});
  obj.inners()[0].push_back(Location{16, 14});
  return obj;
}


TEST(Polygon, equalsOperator) {
  osm2ttl::geometry::Polygon o1;
  o1.outer().reserve(4);
  o1.outer().push_back(Location{10, 10});
  o1.outer().push_back(Location{10, 20});
  o1.outer().push_back(Location{20, 20});
  o1.outer().push_back(Location{20, 10});
  o1.inners().resize(1);
  o1.inners()[0].reserve(3);
  o1.inners()[0].push_back(Location{14, 14});
  o1.inners()[0].push_back(Location{14, 16});
  o1.inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::Polygon o2;
  o2.outer().reserve(4);
  o2.outer().push_back(Location{10, 10});
  o2.outer().push_back(Location{20, 20});
  o2.outer().push_back(Location{10, 20});
  o2.outer().push_back(Location{20, 10});
  o2.inners().resize(1);
  o2.inners()[0].reserve(3);
  o2.inners()[0].push_back(Location{14, 14});
  o2.inners()[0].push_back(Location{14, 16});
  o2.inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::Polygon o3;
  o3.outer().reserve(3);
  o3.outer().push_back(Location{10, 10});
  o3.outer().push_back(Location{10, 20});
  o3.outer().push_back(Location{20, 20});
  o3.inners().resize(1);
  o3.inners()[0].reserve(3);
  o3.inners()[0].push_back(Location{14, 14});
  o3.inners()[0].push_back(Location{14, 16});
  o3.inners()[0].push_back(Location{16, 14});

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

TEST(Polygon, notEqualsOperator) {
  osm2ttl::geometry::Polygon o1;
  o1.outer().reserve(4);
  o1.outer().push_back(Location{10, 10});
  o1.outer().push_back(Location{10, 20});
  o1.outer().push_back(Location{20, 20});
  o1.outer().push_back(Location{20, 10});
  o1.inners().resize(1);
  o1.inners()[0].reserve(3);
  o1.inners()[0].push_back(Location{14, 14});
  o1.inners()[0].push_back(Location{14, 16});
  o1.inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::Polygon o2;
  o2.outer().reserve(4);
  o2.outer().push_back(Location{10, 10});
  o2.outer().push_back(Location{20, 20});
  o2.outer().push_back(Location{10, 20});
  o2.outer().push_back(Location{20, 10});
  o2.inners().resize(1);
  o2.inners()[0].reserve(3);
  o2.inners()[0].push_back(Location{14, 14});
  o2.inners()[0].push_back(Location{14, 16});
  o2.inners()[0].push_back(Location{16, 14});

  osm2ttl::geometry::Polygon o3;
  o3.outer().reserve(3);
  o3.outer().push_back(Location{10, 10});
  o3.outer().push_back(Location{10, 20});
  o3.outer().push_back(Location{20, 20});
  o3.inners().resize(1);
  o3.inners()[0].reserve(3);
  o3.inners()[0].push_back(Location{14, 14});
  o3.inners()[0].push_back(Location{14, 16});
  o3.inners()[0].push_back(Location{16, 14});

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

TEST(Polygon, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Polygon origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Polygon origFilledObject = getFilledObject();
  osm2ttl::geometry::Polygon loadedDefaultObject;
  osm2ttl::geometry::Polygon loadedFilledObject;

  // Store and load
  boost::archive::binary_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.outer().size(),
            loadedDefaultObject.outer().size());
  ASSERT_EQ(origDefaultObject.inners().size(),
            loadedDefaultObject.inners().size());
  ASSERT_EQ(origFilledObject.outer().size(), loadedFilledObject.outer().size());
  for (size_t j = 0; j < origFilledObject.outer().size(); ++j) {
    ASSERT_EQ(origFilledObject.outer()[j].x(),
              loadedFilledObject.outer()[j].x());
    ASSERT_EQ(origFilledObject.outer()[j].y(),
              loadedFilledObject.outer()[j].y());
  }
  for (size_t k = 0; k < origFilledObject.inners().size(); ++k) {
    ASSERT_EQ(origFilledObject.inners()[k].size(),
              loadedFilledObject.inners()[k].size());
    for (size_t j = 0; j < origFilledObject.inners()[k].size(); ++j) {
      ASSERT_EQ(origFilledObject.inners()[k][j].x(),
                loadedFilledObject.inners()[k][j].x());
      ASSERT_EQ(origFilledObject.inners()[k][j].y(),
                loadedFilledObject.inners()[k][j].y());
    }
  }
}

TEST(Polygon, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Polygon origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Polygon origFilledObject = getFilledObject();
  osm2ttl::geometry::Polygon loadedDefaultObject;
  osm2ttl::geometry::Polygon loadedFilledObject;

  // Store and load
  boost::archive::text_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.outer().size(),
            loadedDefaultObject.outer().size());
  ASSERT_EQ(origDefaultObject.inners().size(),
            loadedDefaultObject.inners().size());
  ASSERT_EQ(origFilledObject.outer().size(), loadedFilledObject.outer().size());
  for (size_t j = 0; j < origFilledObject.outer().size(); ++j) {
    ASSERT_EQ(origFilledObject.outer()[j].x(),
              loadedFilledObject.outer()[j].x());
    ASSERT_EQ(origFilledObject.outer()[j].y(),
              loadedFilledObject.outer()[j].y());
  }
  for (size_t k = 0; k < origFilledObject.inners().size(); ++k) {
    ASSERT_EQ(origFilledObject.inners()[k].size(),
              loadedFilledObject.inners()[k].size());
    for (size_t j = 0; j < origFilledObject.inners()[k].size(); ++j) {
      ASSERT_EQ(origFilledObject.inners()[k][j].x(),
                loadedFilledObject.inners()[k][j].x());
      ASSERT_EQ(origFilledObject.inners()[k][j].y(),
                loadedFilledObject.inners()[k][j].y());
    }
  }
}

}  // namespace osm2ttl::geometry