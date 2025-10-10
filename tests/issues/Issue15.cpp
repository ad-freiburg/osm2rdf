// Copyright 2022, University of Freiburg
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

#include <string>
#include <vector>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/OsmiumHandler.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/Output.h"

// ____________________________________________________________________________
TEST(Issue15, Relation_8291361_expected) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.noGeometricRelations = true;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.input = "tests/issues/issue15_osmrel_8291361.xml";
  config.simplifyWKT = 0;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::FactHandler<osm2rdf::ttl::format::QLEVER> factHandler(config, &writer);
  osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER> geomHandler(config, &writer);

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &factHandler, &geomHandler};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:2 dumped: 2 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:207 dumped: 207 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:1 dumped: 1 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:47 dumped: 47 geometry: 0\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osm2rdfgeom:osmrel_8291361 geo:asWKT \"MULTIPOLYGON(((14"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(Issue15, Relation_8291361_failed) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.noGeometricRelations = true;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.input = "tests/issues/issue15_osmrel_8291361.xml";

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::FactHandler<osm2rdf::ttl::format::QLEVER> factHandler(config, &writer);
  osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER> geomHandler(config, &writer);

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &factHandler, &geomHandler};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:2 dumped: 2 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:207 dumped: 207 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:1 dumped: 1 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:47 dumped: 47 geometry: 0\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osm2rdfgeom:osmrel_8291361 geo:asWKT \"MULTIPOLYGON(((14"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(Issue15, Way_201387026_expected) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.noGeometricRelations = true;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.input = "tests/issues/issue15_osmway_201387026.xml";
  config.simplifyWKT = 0;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::FactHandler<osm2rdf::ttl::format::QLEVER> factHandler(config, &writer);
  osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER> geomHandler(config, &writer);

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &factHandler, &geomHandler};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:1 dumped: 1 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:1498 dumped: 1498 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 0\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osm2rdfgeom:osmway_201387026 geo:asWKT \"POLYGON((1"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}

// ____________________________________________________________________________
TEST(Issue15, Way_201387026_failed) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.noGeometricRelations = true;
  config.output = "";
  config.numThreads = 1;  // set to one to avoid concurrency issues with the
                          // stringstream read buffer
  config.outputCompress = osm2rdf::config::NONE;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;
  config.input = "tests/issues/issue15_osmway_201387026.xml";

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::FactHandler<osm2rdf::ttl::format::QLEVER> factHandler(config, &writer);
  osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER> geomHandler(config, &writer);

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &factHandler, &geomHandler};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:1 dumped: 1 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:1498 dumped: 1498 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 0\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osm2rdfgeom:osmway_201387026 geo:asWKT \"POLYGON((1"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
}
