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
#include <regex>

#include "gtest/gtest.h"

namespace osm2rdf::util {

const static std::regex regTs("\\[[0-9,.:\\s-]+\\] ");

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
      "[=====================================================] 100%    \r\n",
      buffer.str().substr(26));
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
      "[=====================================================] 100%    \r\n",
      buffer.str().substr(26));
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
      "[=====================================================] 100%    \r\n",
      buffer.str().substr(26));
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

  // remove timestamps
  std::string clean = std::regex_replace(buffer.str(), regTs, "");

  ASSERT_EQ(
      "[==========================>                          ]  50%    "
      "\r[=====================================================] 100%    \r\n",
      clean);
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

  // remove timestamps
  std::string clean = std::regex_replace(buffer.str(), regTs, "");

  ASSERT_EQ(
      "[=====>                                               ]  10%    "
      "\r[==========>                                          ]  20%    "
      "\r[===============>                                     ]  30%    "
      "\r[=====================>                               ]  40%    "
      "\r[==========================>                          ]  50%    "
      "\r[=====================================>               ]  70%    "
      "\r[===============================================>     ]  90%    "
      "\r[=====================================================] 100%    \r\n",
      clean);
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

  // remove timestamps
  std::string clean = std::regex_replace(buffer.str(), regTs, "");

  ASSERT_EQ(
      "[>                                                    ]   0%    \r[==>  "
      "                                                ]   4%    \r[====>      "
      "                                          ]   9%    \r[=============>   "
      "                                    ]  25%    "
      "\r[==========================>                          ]  50%    "
      "\r[=======================================>             ]  75%    "
      "\r[===============================================>     ]  90%    "
      "\r[================================================>    ]  91%    "
      "\r[================================================>    ]  92%    "
      "\r[=================================================>   ]  93%    "
      "\r[=================================================>   ]  94%    "
      "\r[==================================================>  ]  95%    "
      "\r[==================================================>  ]  96%    "
      "\r[===================================================> ]  97%    "
      "\r[===================================================> ]  98%    "
      "\r[====================================================>]  99%    "
      "\r[=====================================================] 100%    \r\n",
      clean);
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

  // remove timestamps
  std::string clean = std::regex_replace(buffer.str(), regTs, "");

  std::cerr.rdbuf(sbuf);
  ASSERT_EQ(
      "[>                                                    ]   0%    "
      "\r[=====>                                               ]  10%    "
      "\r[==========>                                          ]  20%    "
      "\r[===============>                                     ]  30%    "
      "\r[=====================>                               ]  40%    "
      "\r[==========================>                          ]  50%    "
      "\r[===============================>                     ]  60%    "
      "\r[=====================================>               ]  70%    "
      "\r[==========================================>          ]  80%    "
      "\r[===============================================>     ]  90%    "
      "\r[=====================================================] 100%    \r\n",
      clean);
}

}  // namespace osm2rdf::util
