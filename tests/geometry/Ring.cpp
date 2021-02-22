// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Ring.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Ring getDefaultObject() { return osm2ttl::geometry::Ring(); }

osm2ttl::geometry::Ring getFilledObject() {
  osm2ttl::geometry::Ring obj;
  obj.push_back(Location{0, 0});
  obj.push_back(Location{5, 0});
  obj.push_back(Location{0, 5});
  obj.push_back(Location{10, 10});
  return obj;
}


TEST(Ring, equalsOperator) {
  osm2ttl::geometry::Ring o1;
  o1.push_back(Location{0, 0});
  o1.push_back(Location{5, 0});
  o1.push_back(Location{0, 5});

  osm2ttl::geometry::Ring o2;
  o2.push_back(Location{0, 0});
  o2.push_back(Location{0, 5});
  o2.push_back(Location{5, 0});

  osm2ttl::geometry::Ring o3;
  o3.push_back(Location{0, 0});
  o3.push_back(Location{0, 5});

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

TEST(Ring, notEqualsOperator) {
  osm2ttl::geometry::Ring o1;
  o1.push_back(Location{0, 0});
  o1.push_back(Location{5, 0});
  o1.push_back(Location{0, 5});

  osm2ttl::geometry::Ring o2;
  o2.push_back(Location{0, 0});
  o2.push_back(Location{0, 5});
  o2.push_back(Location{5, 0});

  osm2ttl::geometry::Ring o3;
  o3.push_back(Location{0, 0});
  o3.push_back(Location{0, 5});

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

TEST(Ring, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Ring origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Ring origFilledObject = getFilledObject();
  osm2ttl::geometry::Ring loadedDefaultObject;
  osm2ttl::geometry::Ring loadedFilledObject;

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

TEST(Ring, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Ring origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Ring origFilledObject = getFilledObject();
  osm2ttl::geometry::Ring loadedDefaultObject;
  osm2ttl::geometry::Ring loadedFilledObject;

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