// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2rdf/osm/Node.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_Node, FromNode) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};
  ASSERT_EQ(42, n.id());

  ASSERT_DOUBLE_EQ(7.51, n.geom().x());
  ASSERT_DOUBLE_EQ(48.0, n.geom().y());

  ASSERT_EQ(0, n.tags().size());
}

// ____________________________________________________________________________
TEST(OSM_Node, FromNodeWithTags) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node n{osmiumBuffer.get<osmium::Node>(0)};
  ASSERT_EQ(42, n.id());

  ASSERT_DOUBLE_EQ(7.51, n.geom().x());
  ASSERT_DOUBLE_EQ(48.0, n.geom().y());

  ASSERT_EQ(1, n.tags().size());
  ASSERT_EQ(0, n.tags().count("tag"));
  ASSERT_EQ(1, n.tags().count("city"));
  ASSERT_STREQ("Freiburg", n.tags().at("city").c_str());
}

// ____________________________________________________________________________
TEST(OSM_Node, equalsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer1, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  osmium::builder::add_node(
      osmiumBuffer2, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));

  osmium::builder::add_node(
      osmiumBuffer3, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.52, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node o1{osmiumBuffer1.get<osmium::Node>(0)};
  const osm2rdf::osm::Node o2{osmiumBuffer2.get<osmium::Node>(0)};
  const osm2rdf::osm::Node o3{osmiumBuffer3.get<osmium::Node>(0)};

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
TEST(OSM_Node, notEqualsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer1, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  osmium::builder::add_node(
      osmiumBuffer2, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)));

  osmium::builder::add_node(
      osmiumBuffer3, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.52, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node o1{osmiumBuffer1.get<osmium::Node>(0)};
  const osm2rdf::osm::Node o2{osmiumBuffer2.get<osmium::Node>(0)};
  const osm2rdf::osm::Node o3{osmiumBuffer3.get<osmium::Node>(0)};

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
TEST(OSM_Node, serializationBinary) {
  std::stringstream boostBuffer;

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node src{osmiumBuffer.get<osmium::Node>(0)};

  osm2rdf::osm::Node dst;

  // Store and load
  boost::archive::binary_oarchive oa(boostBuffer);
  oa << src;
  // std::cerr << boostBuffer.str() << std::endl;
  boost::archive::binary_iarchive ia(boostBuffer);
  ia >> dst;

  // Compare
  ASSERT_TRUE(src == dst);
}

// ____________________________________________________________________________
TEST(OSM_Node, serializationText) {
  std::stringstream boostBuffer;

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Node src{osmiumBuffer.get<osmium::Node>(0)};

  osm2rdf::osm::Node dst;

  // Store and load
  boost::archive::text_oarchive oa(boostBuffer);
  oa << src;
  // std::cerr << boostBuffer.str() << std::endl;
  boost::archive::text_iarchive ia(boostBuffer);
  ia >> dst;

  // Compare
  ASSERT_TRUE(src == dst);
}

}  // namespace osm2rdf::osm