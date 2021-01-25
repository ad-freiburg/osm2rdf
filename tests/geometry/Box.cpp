// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/geometry/Box.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"

namespace osm2ttl::geometry {

osm2ttl::geometry::Box getDefaultObject() { return osm2ttl::geometry::Box(); }

osm2ttl::geometry::Box getFilledObject() {
  osm2ttl::geometry::Box obj;
  obj.min_corner() = Location{50, 50};
  obj.max_corner() = Location{200, 200};
  return obj;
}

TEST(Box, serializationBinary) {
  std::stringstream buffer;

  osm2ttl::geometry::Box origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Box origFilledObject = getFilledObject();
  osm2ttl::geometry::Box loadedDefaultObject;
  osm2ttl::geometry::Box loadedFilledObject;

  // Store and load
  boost::archive::binary_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.min_corner().x(),
            loadedDefaultObject.min_corner().x());
  ASSERT_EQ(origDefaultObject.min_corner().y(),
            loadedDefaultObject.min_corner().y());
  ASSERT_EQ(origDefaultObject.max_corner().x(),
            loadedDefaultObject.max_corner().x());
  ASSERT_EQ(origDefaultObject.max_corner().y(),
            loadedDefaultObject.max_corner().y());
  ASSERT_EQ(origFilledObject.min_corner().x(),
            loadedFilledObject.min_corner().x());
  ASSERT_EQ(origFilledObject.min_corner().y(),
            loadedFilledObject.min_corner().y());
  ASSERT_EQ(origFilledObject.max_corner().x(),
            loadedFilledObject.max_corner().x());
  ASSERT_EQ(origFilledObject.max_corner().y(),
            loadedFilledObject.max_corner().y());
}

TEST(Box, serializationText) {
  std::stringstream buffer;

  osm2ttl::geometry::Box origDefaultObject = getDefaultObject();
  osm2ttl::geometry::Box origFilledObject = getFilledObject();
  osm2ttl::geometry::Box loadedDefaultObject;
  osm2ttl::geometry::Box loadedFilledObject;

  // Store and load
  boost::archive::text_oarchive oa(buffer);
  oa << origDefaultObject;
  oa << origFilledObject;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(buffer);
  ia >> loadedDefaultObject;
  ia >> loadedFilledObject;

  // Compare
  ASSERT_EQ(origDefaultObject.min_corner().x(),
            loadedDefaultObject.min_corner().x());
  ASSERT_EQ(origDefaultObject.min_corner().y(),
            loadedDefaultObject.min_corner().y());
  ASSERT_EQ(origDefaultObject.max_corner().x(),
            loadedDefaultObject.max_corner().x());
  ASSERT_EQ(origDefaultObject.max_corner().y(),
            loadedDefaultObject.max_corner().y());
  ASSERT_EQ(origFilledObject.min_corner().x(),
            loadedFilledObject.min_corner().x());
  ASSERT_EQ(origFilledObject.min_corner().y(),
            loadedFilledObject.min_corner().y());
  ASSERT_EQ(origFilledObject.max_corner().x(),
            loadedFilledObject.max_corner().x());
  ASSERT_EQ(origFilledObject.max_corner().y(),
            loadedFilledObject.max_corner().y());
}

}  // namespace osm2ttl::geometry