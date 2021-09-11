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

#include "osm2rdf/osm/RelationMember.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"
#include "osm2rdf/osm/Relation.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_RelationMember, FromRelationWithMembers) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, ""),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(0, r.tags().size());

  ASSERT_EQ(2, r.members().size());
  ASSERT_EQ(osm2rdf::osm::RelationMemberType::NODE, r.members().at(0).type());
  ASSERT_EQ(1, r.members().at(0).id());
  ASSERT_EQ("member", r.members().at(0).role());
  ASSERT_EQ(osm2rdf::osm::RelationMemberType::WAY, r.members().at(1).type());
  ASSERT_EQ(1, r.members().at(1).id());
  ASSERT_EQ("outer", r.members().at(1).role());
}

// ____________________________________________________________________________
TEST(OSM_RelationMember, equalsOperator) {
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
      osmium::builder::attr::_member(osmium::item_type::node, 1, ""),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_member(osmium::item_type::changeset, 1, "foo"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{osmiumBuffer1.get<osmium::Relation>(0)};
  const osm2rdf::osm::RelationMember o1 = r.members().at(0);
  const osm2rdf::osm::RelationMember o2 = r.members().at(1);
  const osm2rdf::osm::RelationMember o3 = r.members().at(2);

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
TEST(OSM_RelationMember, notEqualsOperator) {
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
      osmium::builder::attr::_member(osmium::item_type::node, 1, ""),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_member(osmium::item_type::changeset, 1, "foo"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{osmiumBuffer1.get<osmium::Relation>(0)};
  const osm2rdf::osm::RelationMember o1 = r.members().at(0);
  const osm2rdf::osm::RelationMember o2 = r.members().at(1);
  const osm2rdf::osm::RelationMember o3 = r.members().at(2);

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
TEST(OSM_RelationMember, serializationBinary) {
  std::stringstream boostBuffer;

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
      osmium::builder::attr::_member(osmium::item_type::node, 1, ""),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_member(osmium::item_type::changeset, 1, "foo"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{osmiumBuffer1.get<osmium::Relation>(0)};
  const osm2rdf::osm::RelationMember s1 = r.members().at(0);
  const osm2rdf::osm::RelationMember s2 = r.members().at(1);
  const osm2rdf::osm::RelationMember s3 = r.members().at(2);
  osm2rdf::osm::RelationMember d1;
  osm2rdf::osm::RelationMember d2;
  osm2rdf::osm::RelationMember d3;

  // Store and load
  boost::archive::binary_oarchive oa(boostBuffer);
  oa << s1;
  oa << s2;
  oa << s3;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::binary_iarchive ia(boostBuffer);
  ia >> d1;
  ia >> d2;
  ia >> d3;

  // Compare
  ASSERT_TRUE(s1 == d1);
  ASSERT_TRUE(s2 == d2);
  ASSERT_TRUE(s3 == d3);
}

// ____________________________________________________________________________
TEST(OSM_RelationMember, serializationText) {
  std::stringstream boostBuffer;

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
      osmium::builder::attr::_member(osmium::item_type::node, 1, ""),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"),
      osmium::builder::attr::_member(osmium::item_type::changeset, 1, "foo"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{osmiumBuffer1.get<osmium::Relation>(0)};
  const osm2rdf::osm::RelationMember s1 = r.members().at(0);
  const osm2rdf::osm::RelationMember s2 = r.members().at(1);
  const osm2rdf::osm::RelationMember s3 = r.members().at(2);
  osm2rdf::osm::RelationMember d1;
  osm2rdf::osm::RelationMember d2;
  osm2rdf::osm::RelationMember d3;

  // Store and load
  boost::archive::text_oarchive oa(boostBuffer);
  oa << s1;
  oa << s2;
  oa << s3;
  // std::cerr << buffer.str() << std::endl;
  boost::archive::text_iarchive ia(boostBuffer);
  ia >> d1;
  ia >> d2;
  ia >> d3;

  // Compare
  ASSERT_TRUE(s1 == d1);
  ASSERT_TRUE(s2 == d2);
  ASSERT_TRUE(s3 == d3);
}

}  // namespace osm2rdf::osm
