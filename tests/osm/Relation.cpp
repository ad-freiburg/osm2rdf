// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Relation.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

TEST(Relation, FromRelation) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(buffer, osmium::builder::attr::_id(42));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(0, r.tags().size());

  ASSERT_EQ(0, r.members().size());
}

TEST(Relation, FromRelationWithTags) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(1, r.tags().size());
  ASSERT_EQ(0, r.tags().count("tag"));
  ASSERT_EQ(1, r.tags().count("city"));
  ASSERT_STREQ("Freiburg", r.tags().at("city").c_str());

  ASSERT_EQ(0, r.members().size());
}

TEST(Relation, FromRelationWithMembers) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(0, r.tags().size());

  ASSERT_EQ(2, r.members().size());
  ASSERT_EQ(osm2ttl::osm::RelationMemberType::NODE, r.members().at(0).type());
  ASSERT_EQ(1, r.members().at(0).id());
  ASSERT_EQ("label", r.members().at(0).role());
  ASSERT_EQ(osm2ttl::osm::RelationMemberType::WAY, r.members().at(1).type());
  ASSERT_EQ(1, r.members().at(1).id());
  ASSERT_EQ("outer", r.members().at(1).role());
}

}  // namespace osm2ttl::osm