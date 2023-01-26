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

#include "osm2rdf/osm/Area.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "gtest/gtest.h"
#include "osmium/builder/attr.hpp"
#include "osmium/builder/osm_object_builder.hpp"

namespace osm2rdf::osm {

// ____________________________________________________________________________
TEST(OSM_Area, FromAreaVirtualWay) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();
  ASSERT_EQ(42, a.id());
  ASSERT_EQ(21, a.objId());
  ASSERT_TRUE(a.fromWay());
  ASSERT_NEAR(a.envelopeArea(), a.geomArea(), 0.01);
  ASSERT_NEAR(48.0, a.envelope().min_corner().x(), 0.01);
  ASSERT_NEAR(7.51, a.envelope().min_corner().y(), 0.01);
  ASSERT_NEAR(48.1, a.envelope().max_corner().x(), 0.01);
  ASSERT_NEAR(7.61, a.envelope().max_corner().y(), 0.01);
}

// ____________________________________________________________________________
TEST(OSM_Area, FromAreaVirtualRelation) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(43),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();
  ASSERT_EQ(43, a.id());
  ASSERT_EQ(21, a.objId());
  ASSERT_FALSE(a.fromWay());
  ASSERT_NEAR(a.envelopeArea(), a.geomArea(), 0.01);
  ASSERT_NEAR(48.0, a.envelope().min_corner().x(), 0.01);
  ASSERT_NEAR(7.51, a.envelope().min_corner().y(), 0.01);
  ASSERT_NEAR(48.1, a.envelope().max_corner().x(), 0.01);
  ASSERT_NEAR(7.61, a.envelope().max_corner().y(), 0.01);
}

// ____________________________________________________________________________
TEST(OSM_Area, BoundaryWithAdminLevel) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(
      osmiumBuffer, osmium::builder::attr::_id(43),
      osmium::builder::attr::_outer_ring({
          {1, {48.0, 7.51}},
          {2, {48.0, 7.61}},
          {3, {48.1, 7.61}},
          {4, {48.1, 7.51}},
          {1, {48.0, 7.51}},
      }),
      osmium::builder::attr::_tag("boundary", "administrative"),
      osmium::builder::attr::_tag("admin_level", "4"));

  // Create osm2rdf object from osmium object
  osm2rdf::osm::Area a{osmiumBuffer.get<osmium::Area>(0)};
  a.finalize();
  ASSERT_EQ(43, a.id());
  ASSERT_EQ(21, a.objId());
  ASSERT_FALSE(a.fromWay());
  ASSERT_NEAR(a.envelopeArea(), a.geomArea(), 0.01);
  ASSERT_NEAR(48.0, a.envelope().min_corner().x(), 0.01);
  ASSERT_NEAR(7.51, a.envelope().min_corner().y(), 0.01);
  ASSERT_NEAR(48.1, a.envelope().max_corner().x(), 0.01);
  ASSERT_NEAR(7.61, a.envelope().max_corner().y(), 0.01);
}

// ____________________________________________________________________________
TEST(OSM_Area, equalsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.62}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  const osm2rdf::osm::Area o1{osmiumBuffer1.get<osmium::Area>(0)};
  const osm2rdf::osm::Area o2{osmiumBuffer2.get<osmium::Area>(0)};
  const osm2rdf::osm::Area o3{osmiumBuffer3.get<osmium::Area>(0)};

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
TEST(OSM_Area, notEqualsOperator) {
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer1{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer2{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::memory::Buffer osmiumBuffer3{initial_buffer_size,
                                       osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer1, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer2, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {1, {48.0, 7.51}},
                            }));
  osmium::builder::add_area(osmiumBuffer3, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.62}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));
  const osm2rdf::osm::Area o1{osmiumBuffer1.get<osmium::Area>(0)};
  const osm2rdf::osm::Area o2{osmiumBuffer2.get<osmium::Area>(0)};
  const osm2rdf::osm::Area o3{osmiumBuffer3.get<osmium::Area>(0)};

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
TEST(OSM_Area, serializationBinary) {
  std::stringstream boostBuffer;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{osmiumBuffer.get<osmium::Area>(0)};

  osm2rdf::osm::Area dst;

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
TEST(OSM_Area, serializationText) {
  std::stringstream boostBuffer;
  // Create osmium object
  const size_t initial_buffer_size = 10000;
  osmium::memory::Buffer osmiumBuffer{initial_buffer_size,
                                      osmium::memory::Buffer::auto_grow::yes};
  osmium::builder::add_area(osmiumBuffer, osmium::builder::attr::_id(42),
                            osmium::builder::attr::_outer_ring({
                                {1, {48.0, 7.51}},
                                {2, {48.0, 7.61}},
                                {3, {48.1, 7.61}},
                                {4, {48.1, 7.51}},
                                {1, {48.0, 7.51}},
                            }));

  // Create osm2rdf object from osmium object
  const osm2rdf::osm::Area src{osmiumBuffer.get<osmium::Area>(0)};

  osm2rdf::osm::Area dst;

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
