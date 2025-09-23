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

#include "osm2rdf/osm/Relation.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_Relation, FromRelation) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(buffer, osmium::builder::attr::_id(42));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(0, r.tags().size());

  ASSERT_EQ(0, r.members().size());
}

// ____________________________________________________________________________
TEST(OSM_Relation, FromRelationWithTags) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(1, r.tags().size());
  ASSERT_STREQ("Freiburg", r.tags()["city"]);

  ASSERT_EQ(0, r.members().size());
}

// ____________________________________________________________________________
TEST(OSM_Relation, FromRelationWithMembers) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_relation(
      buffer, osmium::builder::attr::_id(42),
      osmium::builder::attr::_member(osmium::item_type::node, 1, "label"),
      osmium::builder::attr::_member(osmium::item_type::way, 1, "outer"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(0, r.tags().size());

  ASSERT_EQ(2, r.members().size());
  ASSERT_EQ(osmium::item_type::node, r.members().begin()->type());
  ASSERT_EQ(1, r.members().begin()->positive_ref());
  ASSERT_STREQ("label", r.members().begin()->role());
  ASSERT_EQ(osmium::item_type::way, (++r.members().begin())->type());
  ASSERT_EQ(1, (r.members().begin()++)->positive_ref());
  ASSERT_STREQ("outer", (++r.members().begin())->role());
}

// ____________________________________________________________________________
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

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Relation r{buffer.get<osmium::Relation>(0)};
  ASSERT_EQ(42, r.id());

  ASSERT_EQ(1, r.tags().size());
  ASSERT_STREQ("Freiburg", r.tags()["city"]);

  ASSERT_EQ(2, r.members().size());
  ASSERT_EQ(osmium::item_type::node, r.members().begin()->type());
  ASSERT_EQ(1, r.members().begin()->positive_ref());
  ASSERT_STREQ("label", r.members().begin()->role());
  ASSERT_EQ(osmium::item_type::way, (++r.members().begin())->type());
  ASSERT_EQ(1, (++r.members().begin())->positive_ref());
  ASSERT_STREQ("outer", (++r.members().begin())->role());
}

}  // namespace osm2rdf::osm
