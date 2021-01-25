// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Area.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Area getDefaultObject() { return osm2ttl::geometry::Area(); }

osm2ttl::geometry::Area getFilledObject() {
  osm2ttl::geometry::Area obj;
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

TEST(Area, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Area origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Area origFilledObject = getFilledObject();
  osm2ttl::geometry::Area loadedDefaultObject;
  osm2ttl::geometry::Area loadedFilledObject;

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
  for (size_t i = 0; i < origFilledObject.size(); ++i) {
    ASSERT_EQ(origFilledObject[i].outer().size(),
              loadedFilledObject[i].outer().size());
    for (size_t j = 0; j < origFilledObject[i].outer().size(); ++j) {
      ASSERT_EQ(origFilledObject[i].outer()[j].x(),
                loadedFilledObject[i].outer()[j].x());
      ASSERT_EQ(origFilledObject[i].outer()[j].y(),
                loadedFilledObject[i].outer()[j].y());
    }
    for (size_t k = 0; k < origFilledObject[i].inners().size(); ++k) {
      ASSERT_EQ(origFilledObject[i].inners()[k].size(),
                loadedFilledObject[i].inners()[k].size());
      for (size_t j = 0; j < origFilledObject[i].inners()[k].size(); ++j) {
        ASSERT_EQ(origFilledObject[i].inners()[k][j].x(),
                  loadedFilledObject[i].inners()[k][j].x());
        ASSERT_EQ(origFilledObject[i].inners()[k][j].y(),
                  loadedFilledObject[i].inners()[k][j].y());
      }
    }
  }
}

TEST(Area, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Area origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Area origFilledObject = getFilledObject();
  osm2ttl::geometry::Area loadedDefaultObject;
  osm2ttl::geometry::Area loadedFilledObject;

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
  for (size_t i = 0; i < origFilledObject.size(); ++i) {
    ASSERT_EQ(origFilledObject[i].outer().size(),
              loadedFilledObject[i].outer().size());
    for (size_t j = 0; j < origFilledObject[i].outer().size(); ++j) {
      ASSERT_EQ(origFilledObject[i].outer()[j].x(),
                loadedFilledObject[i].outer()[j].x());
      ASSERT_EQ(origFilledObject[i].outer()[j].y(),
                loadedFilledObject[i].outer()[j].y());
    }
    for (size_t k = 0; k < origFilledObject[i].inners().size(); ++k) {
      ASSERT_EQ(origFilledObject[i].inners()[k].size(),
                loadedFilledObject[i].inners()[k].size());
      for (size_t j = 0; j < origFilledObject[i].inners()[k].size(); ++j) {
        ASSERT_EQ(origFilledObject[i].inners()[k][j].x(),
                  loadedFilledObject[i].inners()[k][j].x());
        ASSERT_EQ(origFilledObject[i].inners()[k][j].y(),
                  loadedFilledObject[i].inners()[k][j].y());
      }
    }
  }
}

}  // namespace osm2ttl::geometry