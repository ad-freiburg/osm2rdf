// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Area.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

TEST(Area, FromArea) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_outer_ring({
                                        {1, {48.0, 7.51}},
                                        {2, {48.0, 7.61}},
                                        {3, {48.1, 7.61}},
                                        {4, {48.1, 7.51}},
                                        {1, {48.0, 7.51}},
                                    }));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Area a{buffer.get<osmium::Area>(0)};
  ASSERT_EQ(42, a.id());
  ASSERT_NEAR(a.envelopeArea(), a.geomArea(), 0.01);
}