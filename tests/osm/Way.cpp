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

#include "osm2rdf/osm/Way.h"

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_Way, FromWay) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_EQ(42, w.id());
  ASSERT_FALSE(w.closed());

  ASSERT_EQ(0, w.tags().size());

  ASSERT_EQ(2, w.nodes().size());
  ASSERT_EQ(1, w.nodes().at(0).id());
  ASSERT_EQ(2, w.nodes().at(1).id());

  ASSERT_EQ(2, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().getLowerLeft().getX());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().getLowerLeft().getY());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().getUpperRight().getX());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().getUpperRight().getY());
}

// ____________________________________________________________________________
TEST(OSM_Way, FromWayWithTags) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
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
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().getLowerLeft().getX());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().getLowerLeft().getY());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().getUpperRight().getX());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().getUpperRight().getY());
}

// ____________________________________________________________________________
TEST(OSM_Way, FromClosedWay) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                               {1, {48.0, 7.51}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_EQ(42, w.id());
  ASSERT_TRUE(w.closed());

  ASSERT_EQ(0, w.tags().size());

  ASSERT_EQ(3, w.nodes().size());
  ASSERT_EQ(1, w.nodes().at(0).id());
  ASSERT_EQ(2, w.nodes().at(1).id());
  ASSERT_EQ(1, w.nodes().at(2).id());

  ASSERT_EQ(3, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(2).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(2).getY());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().getLowerLeft().getX());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().getLowerLeft().getY());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().getUpperRight().getX());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().getUpperRight().getY());
}

// ____________________________________________________________________________
TEST(OSM_Way, FromClosedWayWithDuplicateNodes) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                               {2, {48.1, 7.61}},
                               {2, {48.1, 7.61}},
                               {1, {48.0, 7.51}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
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
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(2).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(2).getY());

  ASSERT_DOUBLE_EQ(48.0, w.envelope().getLowerLeft().getX());
  ASSERT_DOUBLE_EQ(7.51, w.envelope().getLowerLeft().getY());
  ASSERT_DOUBLE_EQ(48.1, w.envelope().getUpperRight().getX());
  ASSERT_DOUBLE_EQ(7.61, w.envelope().getUpperRight().getY());
}

// ____________________________________________________________________________
TEST(OSM_Way, isAreaFalseForClosedWayWithoutArea) {
    // Create osmium object
    const size_t initial_buffer_size = 10000;
    osmium::memory::Buffer buffer{initial_buffer_size,
                                  osmium::memory::Buffer::auto_grow::yes};
    osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                             osmium::builder::attr::_nodes({
                                 {1, {48.0, 7.51}},
                                 {2, {48.1, 7.61}},
                                 {1, {48.0, 7.51}},
                             }));

    // Create osm2rdf object from osmium object
    const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
    ASSERT_TRUE(w.closed());

    ASSERT_FALSE(w.isArea());
}

// ____________________________________________________________________________
TEST(OSM_Way, isAreaFalseForOpenWay) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.0, 7.61}},
                               {1, {48.1, 7.61}},
                               {1, {48.1, 7.51}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_FALSE(w.closed());

  ASSERT_FALSE(w.isArea());
}

// ____________________________________________________________________________
TEST(OSM_Way, isAreaTrueForTriangle) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.0, 7.61}},
                               {1, {48.1, 7.61}},
                               {1, {48.0, 7.51}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_TRUE(w.closed());

  ASSERT_TRUE(w.isArea());
}

// ____________________________________________________________________________
TEST(OSM_Way, isAreaFalseForTriangleMarkedAsNoArea) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer buffer{initial_buffer_size,
                                osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(buffer, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.0, 7.61}},
                               {1, {48.1, 7.61}},
                               {1, {48.0, 7.51}},
                           }),
                           osmium::builder::attr::_tag("area", "no"));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
  ASSERT_TRUE(w.closed());

  ASSERT_FALSE(w.isArea());
}

// ____________________________________________________________________________
TEST(OSM_Way, equalsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer1, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_way(osmiumBuffer2, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.52}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_way(osmiumBuffer3, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way o1{osmiumBuffer1.get<osmium::Way>(0)};
  const osm2rdf::osm::Way o2{osmiumBuffer2.get<osmium::Way>(0)};
  const osm2rdf::osm::Way o3{osmiumBuffer3.get<osmium::Way>(0)};

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
TEST(OSM_Way, notEqualsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_way(osmiumBuffer1, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_way(osmiumBuffer2, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.52}},
                               {2, {48.1, 7.61}},
                           }),
                           osmium::builder::attr::_tag("city", "Freiburg"));
  osmium::builder::add_way(osmiumBuffer3, osmium::builder::attr::_id(42),
                           osmium::builder::attr::_nodes({
                               {1, {48.0, 7.51}},
                               {2, {48.1, 7.61}},
                           }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Way o1{osmiumBuffer1.get<osmium::Way>(0)};
  const osm2rdf::osm::Way o2{osmiumBuffer2.get<osmium::Way>(0)};
  const osm2rdf::osm::Way o3{osmiumBuffer3.get<osmium::Way>(0)};

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

}  // namespace osm2rdf::osm
