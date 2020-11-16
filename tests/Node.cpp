// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "gtest/gtest.h"

#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

#include "osm2ttl/osm/Node.h"

TEST(Node, FromNode) {
  // Create osmium node
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      buffer,
      osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0))
      );

  // Create osm2ttl node from osmium node
  const osm2ttl::osm::Node n{buffer.get<osmium::Node>(0)};
  ASSERT_EQ(42, n.id());
  ASSERT_DOUBLE_EQ(7.51, n.geom().x());
  ASSERT_DOUBLE_EQ(48.0, n.geom().y());
  ASSERT_EQ(0, n.tags().size());
}

TEST(Node, FromNodeWithTags) {
  // Create osmium node
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_node(
      buffer,
      osmium::builder::attr::_id(42),
      osmium::builder::attr::_location(osmium::Location(7.51, 48.0)),
      osmium::builder::attr::_tag("city", "Freiburg")
  );

  // Create osm2ttl node from osmium node
  const osm2ttl::osm::Node n{buffer.get<osmium::Node>(0)};
  ASSERT_EQ(42, n.id());
  ASSERT_DOUBLE_EQ(7.51, n.geom().x());
  ASSERT_DOUBLE_EQ(48.0, n.geom().y());
  ASSERT_EQ(1, n.tags().size());
  ASSERT_EQ(0, n.tags().count("tag"));
  ASSERT_EQ(1, n.tags().count("city"));
  ASSERT_STREQ("Freiburg", n.tags().at("city").c_str());
}