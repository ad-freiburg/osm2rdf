// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/TagList.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

TEST(OSM_TagList, convertTagList) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2ttl object from osmium object
  osm2ttl::osm::TagList tl =
      osm2ttl::osm::convertTagList(osmiumBuffer.get<osmium::Node>(0).tags());

  ASSERT_EQ(1, tl.size());
  ASSERT_EQ("Freiburg", tl["city"]);
}

TEST(OSM_TagList, serializationBinary) {
  std::stringstream boostBuffer;

  osm2ttl::osm::TagList src;
  src["abc"] = "xyz";
  src["def"] = "42";

  osm2ttl::osm::TagList dst;

  // Store and load
  boost::archive::binary_oarchive oa(boostBuffer);
  oa << src;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(boostBuffer);
  ia >> dst;

  // Compare
  ASSERT_TRUE(src == dst);
}

TEST(OSM_TagList, serializationText) {
  std::stringstream boostBuffer;

  osm2ttl::osm::TagList src;
  src["abc"] = "xyz";
  src["def"] = "42";

  osm2ttl::osm::TagList dst;

  // Store and load
  boost::archive::text_oarchive oa(boostBuffer);
  oa << src;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(boostBuffer);
  ia >> dst;

  // Compare
  ASSERT_TRUE(src == dst);
}
}  // namespace osm2ttl::osm
