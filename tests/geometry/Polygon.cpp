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