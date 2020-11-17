// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "gtest/gtest.h"

#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

#include "osm2ttl/osm/Way.h"

TEST(Way, FromWay) {
  // Create osmium node
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(
      buffer,
      osmium::builder::attr::_id(42),
      osmium::builder::attr::_nodes({
                                        {1, {48.0, 7.51}},
                                        {2, {48.1, 7.61}},
                                    })
  );

  // Create osm2ttl node from osmium node
  const osm2ttl::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_EQ(42, w.id());
  ASSERT_FALSE(w.closed());

  ASSERT_EQ(0, w.tags().size());

  ASSERT_EQ(2, w.nodes().size());
  ASSERT_EQ(1, w.nodes().at(0).id());
  ASSERT_EQ(2, w.nodes().at(1).id());

  ASSERT_EQ(2, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).x());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).y());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).x());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).y());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().min_corner().x());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().min_corner().y());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().max_corner().x());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().max_corner().y());
}

TEST(Way, FromWayWithTags) {
  // Create osmium node
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(
      buffer,
      osmium::builder::attr::_id(42),
      osmium::builder::attr::_nodes({
                                        {1, {48.0, 7.51}},
                                        {2, {48.1, 7.61}},
                                    }),
      osmium::builder::attr::_tag("city", "Freiburg")
  );

  // Create osm2ttl node from osmium node
  const osm2ttl::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_EQ(42, w.id());
  ASSERT_FALSE(w.closed());

  ASSERT_EQ(1, w.tags().size());
  ASSERT_EQ(0, w.tags().count("tag"));
  ASSERT_EQ(1, w.tags().count("city"));
  ASSERT_STREQ("Freiburg", w.tags().at("city").c_str());

  ASSERT_EQ(2, w.nodes().size());
  ASSERT_EQ(1, w.nodes().at(0).id());
  ASSERT_EQ(2, w.nodes().at(1).id());

  ASSERT_EQ(2, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).x());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).y());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).x());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).y());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().min_corner().x());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().min_corner().y());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().max_corner().x());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().max_corner().y());
}

TEST(Way, FromClosedWay) {
  // Create osmium node
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(
      buffer,
      osmium::builder::attr::_id(42),
      osmium::builder::attr::_nodes({
                                        {1, {48.0, 7.51}},
                                        {2, {48.1, 7.61}},
                                        {1, {48.0, 7.51}},
                                    })
  );

  // Create osm2ttl node from osmium node
  const osm2ttl::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_EQ(42, w.id());
  ASSERT_TRUE(w.closed());

  ASSERT_EQ(0, w.tags().size());

  ASSERT_EQ(3, w.nodes().size());
  ASSERT_EQ(1, w.nodes().at(0).id());
  ASSERT_EQ(2, w.nodes().at(1).id());
  ASSERT_EQ(1, w.nodes().at(2).id());

  ASSERT_EQ(3, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).x());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).y());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).x());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).y());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(2).x());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(2).y());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().min_corner().x());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().min_corner().y());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().max_corner().x());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().max_corner().y());
}


TEST(Way, FromClosedWayWithDuplicateNodes) {
  // Create osmium node
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(
      buffer,
      osmium::builder::attr::_id(42),
      osmium::builder::attr::_nodes({
                                        {1, {48.0, 7.51}},
                                        {2, {48.1, 7.61}},
                                        {2, {48.1, 7.61}},
                                        {2, {48.1, 7.61}},
                                        {1, {48.0, 7.51}},
                                    })
  );

  // Create osm2ttl node from osmium node
  const osm2ttl::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_EQ(42, w.id());
  ASSERT_TRUE(w.closed());

  ASSERT_EQ(0, w.tags().size());

  ASSERT_EQ(5, w.nodes().size());
  ASSERT_EQ(1, w.nodes().at(0).id());
  ASSERT_EQ(2, w.nodes().at(1).id());
  ASSERT_EQ(2, w.nodes().at(2).id());
  ASSERT_EQ(2, w.nodes().at(3).id());
  ASSERT_EQ(1, w.nodes().at(4).id());

  ASSERT_EQ(3, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).x());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).y());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).x());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).y());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(2).x());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(2).y());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().min_corner().x());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().min_corner().y());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().max_corner().x());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().max_corner().y());
}