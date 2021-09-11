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

#include "osm2rdf/util/ProgressBar.h"

#include <iostream>

#include "gtest/gtest.h"

namespace osm2rdf::util {

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, countWidth) {
  {
    osm2rdf::util::ProgressBar pg{0, false};
    ASSERT_EQ(1, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{1, false};
    ASSERT_EQ(1, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{9, false};
    ASSERT_EQ(1, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{10, false};
    ASSERT_EQ(2, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{99, false};
    ASSERT_EQ(2, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{100, false};
    ASSERT_EQ(3, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{999, false};
    ASSERT_EQ(3, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{1000, false};
    ASSERT_EQ(4, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{10000, false};
    ASSERT_EQ(5, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{100000, false};
    ASSERT_EQ(6, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{1000000, false};
    ASSERT_EQ(7, pg.countWidth());
  }
  {
    osm2rdf::util::ProgressBar pg{10000000, false};
    ASSERT_EQ(8, pg.countWidth());
  }
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, hidden) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{10, false};
  pg.update(2);
  pg.update(4);
  pg.update(6);
  pg.update(8);
  pg.update(10);
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ("", buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, directlyDoneZero) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{0, true};
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[===================================================================] "
      "100% [0/0]\r\n",
      buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, directlyDoneOne) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{1, true};
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[===================================================================] "
      "100% [1/1]\r\n",
      buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, zeroSteps) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{0, true};
  pg.update(0);
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[===================================================================] "
      "100% [0/0]\r\n",
      buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, twoSteps) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{2, true};
  pg.update(1);
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[=================================>                                 ] "
      " 50% [1/2]\r"
      "[===================================================================] "
      "100% [2/2]\r\n",
      buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, tenSteps) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{10, true};
  pg.update(1);
  pg.update(2);
  pg.update(3);
  pg.update(4);
  pg.update(5);
  pg.update(7);
  pg.update(9);
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[======>                                                          ]  "
      "10% [ 1/10]\r"
      "[=============>                                                   ]  "
      "20% [ 2/10]\r"
      "[===================>                                             ]  "
      "30% [ 3/10]\r"
      "[==========================>                                      ]  "
      "40% [ 4/10]\r"
      "[================================>                                ]  "
      "50% [ 5/10]\r"
      "[=============================================>                   ]  "
      "70% [ 7/10]\r"
      "[==========================================================>      ]  "
      "90% [ 9/10]\r"
      "[=================================================================] "
      "100% [10/10]\r\n",
      buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, oneHundredSteps) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{100, true};
  pg.update(0);
  pg.update(4);
  pg.update(9);
  pg.update(25);
  pg.update(50);
  pg.update(75);
  pg.update(90);
  pg.update(91);
  pg.update(92);
  pg.update(93);
  pg.update(94);
  pg.update(95);
  pg.update(96);
  pg.update(97);
  pg.update(98);
  pg.update(99);
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[>                                                              ] "
      "  0% [  0/100]\r"
      "[==>                                                            ] "
      "  4% [  4/100]\r"
      "[=====>                                                         ] "
      "  9% [  9/100]\r"
      "[===============>                                               ] "
      " 25% [ 25/100]\r"
      "[===============================>                               ] "
      " 50% [ 50/100]\r"
      "[===============================================>               ] "
      " 75% [ 75/100]\r"
      "[========================================================>      ] "
      " 90% [ 90/100]\r"
      "[=========================================================>     ] "
      " 91% [ 91/100]\r"
      "[=========================================================>     ] "
      " 92% [ 92/100]\r"
      "[==========================================================>    ] "
      " 93% [ 93/100]\r"
      "[===========================================================>   ] "
      " 94% [ 94/100]\r"
      "[===========================================================>   ] "
      " 95% [ 95/100]\r"
      "[============================================================>  ] "
      " 96% [ 96/100]\r"
      "[=============================================================> ] "
      " 97% [ 97/100]\r"
      "[=============================================================> ] "
      " 98% [ 98/100]\r"
      "[==============================================================>] "
      " 99% [ 99/100]\r"
      "[===============================================================] "
      "100% [100/100]\r\n",
      buffer.str());
}

// ____________________________________________________________________________
TEST(UTIL_ProgressBar, multipleNoChangeSteps) {
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::util::ProgressBar pg{10, true};
  for (size_t i = 0; i <= 10; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      pg.update(i);
    }
  }
  pg.done();

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[>                                                                ] "
      "  0% [ 0/10]\r"
      "[======>                                                          ] "
      " 10% [ 1/10]\r"
      "[=============>                                                   ] "
      " 20% [ 2/10]\r"
      "[===================>                                             ] "
      " 30% [ 3/10]\r"
      "[==========================>                                      ] "
      " 40% [ 4/10]\r"
      "[================================>                                ] "
      " 50% [ 5/10]\r"
      "[=======================================>                         ] "
      " 60% [ 6/10]\r"
      "[=============================================>                   ] "
      " 70% [ 7/10]\r"
      "[====================================================>            ] "
      " 80% [ 8/10]\r"
      "[==========================================================>      ] "
      " 90% [ 9/10]\r"
      "[=================================================================] "
      "100% [10/10]\r\n",
      buffer.str());
}

}  // namespace osm2rdf::util
