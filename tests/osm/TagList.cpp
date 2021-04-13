// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/osm/TagList.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

// ____________________________________________________________________________
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

// ____________________________________________________________________________
TEST(OSM_TagList, convertTagListWithSpaceInKey) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city name", "Freiburg"),
      osmium::builder::attr::_tag("name of city", "Freiburg"));

  // Create osm2ttl object from osmium object
  osm2ttl::osm::TagList tl =
      osm2ttl::osm::convertTagList(osmiumBuffer.get<osmium::Node>(0).tags());

  ASSERT_EQ(2, tl.size());
  ASSERT_EQ("Freiburg", tl["city_name"]);
  ASSERT_EQ("Freiburg", tl["name_of_city"]);
}

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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
