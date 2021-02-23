// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/Relation.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2ttl::osm {

TEST(OSM_Relation, FromRelation) {
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

TEST(OSM_Relation, FromRelationWithTags) {
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

TEST(OSM_Relation, FromRelationWithMembers) {
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

TEST(OSM_Relation, FromRelationWithMembersAndTags) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(1, r.tags().size());
  ASSERT_EQ(0, r.tags().count("tag"));
  ASSERT_EQ(1, r.tags().count("city"));
  ASSERT_STREQ("Freiburg", r.tags().at("city").c_str());

  ASSERT_EQ(2, r.members().size());
  ASSERT_EQ(osm2ttl::osm::RelationMemberType::NODE, r.members().at(0).type());
  ASSERT_EQ(1, r.members().at(0).id());
  ASSERT_EQ("label", r.members().at(0).role());
  ASSERT_EQ(osm2ttl::osm::RelationMemberType::WAY, r.members().at(1).type());
  ASSERT_EQ(1, r.members().at(1).id());
  ASSERT_EQ("outer", r.members().at(1).role());
}

TEST(OSM_Relation, equalsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer1, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_relation(
      osmiumBuffer2, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 2, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_relation(
      osmiumBuffer3, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg i. Brsg."));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation o1{osmiumBuffer1.get<osmium::Relation>(0)};
  const osm2ttl::osm::Relation o2{osmiumBuffer2.get<osmium::Relation>(0)};
  const osm2ttl::osm::Relation o3{osmiumBuffer3.get<osmium::Relation>(0)};

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

TEST(OSM_Relation, notEqualsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer1, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_relation(
      osmiumBuffer2, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 2, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_relation(
      osmiumBuffer3, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg i. Brsg."));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation o1{osmiumBuffer1.get<osmium::Relation>(0)};
  const osm2ttl::osm::Relation o2{osmiumBuffer2.get<osmium::Relation>(0)};
  const osm2ttl::osm::Relation o3{osmiumBuffer3.get<osmium::Relation>(0)};

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

TEST(OSM_Relation, serializationBinary) {
  std::stringstream boostBuffer;

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation src{osmiumBuffer.get<osmium::Relation>(0)};

  osm2ttl::osm::Relation dst;

  // Store and load
  boost::archive::binary_oarchive oa(boostBuffer);
  oa << src;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(boostBuffer);
  ia >> dst;

  // Compare
  ASSERT_TRUE(src == dst);
}

TEST(OSM_Relation, serializationText) {
  std::stringstream boostBuffer;

  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      osmiumBuffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2ttl object from osmium object
  const osm2ttl::osm::Relation src{osmiumBuffer.get<osmium::Relation>(0)};

  osm2ttl::osm::Relation dst;

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