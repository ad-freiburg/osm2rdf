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
  ASSERT_EQ(1, w.nodes()[0].positive_ref());
  ASSERT_EQ(2, w.nodes()[1].positive_ref());

  ASSERT_EQ(2, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());
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
  ASSERT_EQ("Freiburg", w.tags()["city"]);

  ASSERT_EQ(2, w.nodes().size());
  ASSERT_EQ(1, w.nodes()[0].positive_ref());
  ASSERT_EQ(2, w.nodes()[1].positive_ref());

  ASSERT_EQ(2, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());
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
  ASSERT_EQ(1, w.nodes()[0].positive_ref());
  ASSERT_EQ(2, w.nodes()[1].positive_ref());
  ASSERT_EQ(1, w.nodes()[2].positive_ref());

  ASSERT_EQ(3, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(2).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(2).getY());
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
  ASSERT_EQ(1, w.nodes()[0].positive_ref());
  ASSERT_EQ(2, w.nodes()[1].positive_ref());
  ASSERT_EQ(2, w.nodes()[2].positive_ref());
  ASSERT_EQ(2, w.nodes()[3].positive_ref());
  ASSERT_EQ(1, w.nodes()[4].positive_ref());

  ASSERT_EQ(3, w.geom().size());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(0).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(0).getY());
  ASSERT_DOUBLE_EQ(48.1, w.geom().at(1).getX());
  ASSERT_DOUBLE_EQ(7.61, w.geom().at(1).getY());
  ASSERT_DOUBLE_EQ(48.0, w.geom().at(2).getX());
  ASSERT_DOUBLE_EQ(7.51, w.geom().at(2).getY());
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
  osm2rdf::osm::Way w{buffer.get<osmium::Way>(0)};
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

}  // namespace osm2rdf::osm
