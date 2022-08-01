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

#include <iostream>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2rdf/util/Output.h"

namespace osm2rdf::util {

// ____________________________________________________________________________
TEST(Issue28, OpenReadonlyOutputFile) {
  // Capture std::cerr
  std::stringstream buffer;
  std::streambuf* sbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_ISSUES_Issue28", "OpenReadonlyOutputFile");
  config.mergeOutput = OutputMergeMode::MERGE;
  config.outputCompress = false;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  std::ofstream out{output};
  out.close();
  std::filesystem::permissions(output, std::filesystem::perms::owner_read);
  ASSERT_TRUE(std::filesystem::exists(output));
  ASSERT_TRUE(std::filesystem::is_regular_file(output));

  size_t parts = 4;
  osm2rdf::util::Output o1{config, output, parts};
  ASSERT_FALSE(o1.open());
  const std::string res = buffer.str();
  ASSERT_THAT(res, ::testing::HasSubstr("Can't open final output file: " +
                                        std::string(output)));

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));

  // Restore std::err
  std::cerr.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(Issue28, OutputfileTruncatedOnOpenConcatenate) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_ISSUES_Issue28",
                                     "OutputfileTruncatedOnOpenConcatenate");
  config.mergeOutput = OutputMergeMode::CONCATENATE;
  config.outputCompress = false;
  config.outputKeepFiles = true;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  size_t parts = 4;
  osm2rdf::util::Output o{config, output, parts};
  o.open();
  // Write content
  o.write("a", 0);
  o.write("b", 1);
  o.write("c", 2);
  o.write("d", 3);
  o.flush();
  o.close("p", "s");

  // Check content
  {
    std::stringstream resultBuffer;
    std::ifstream resultFile(output);
    ASSERT_TRUE(resultFile.is_open());
    ASSERT_TRUE(resultFile.good());
    resultBuffer << resultFile.rdbuf();
    ASSERT_EQ("pabcds", resultBuffer.str());
    resultFile.close();
    resultBuffer.clear();
  }

  // Reopen file for writing -> this clears the file.
  o.open();
  {
    std::stringstream resultBuffer;
    std::ifstream resultFile(output);
    ASSERT_TRUE(resultFile.is_open());
    ASSERT_TRUE(resultFile.good());
    resultBuffer << resultFile.rdbuf();
    ASSERT_EQ("", resultBuffer.str());
    resultFile.close();
    resultBuffer.clear();
  }

  // Write new content
  o.write("z", 3);
  o.write("y", 2);
  o.write("x", 1);
  o.write("w", 0);
  o.flush();
  o.close("p", "s");

  // Check new content
  {
    std::stringstream resultBuffer;
    std::ifstream resultFile(output);
    ASSERT_TRUE(resultFile.is_open());
    ASSERT_TRUE(resultFile.good());
    resultBuffer << resultFile.rdbuf();
    ASSERT_EQ("pwxyzs", resultBuffer.str());
    resultFile.close();
    resultBuffer.clear();
  }

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));
}

// ____________________________________________________________________________
TEST(Issue28, OutputfileTruncatedOnOpenMerge) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_ISSUES_Issue28",
                                     "OutputfileTruncatedOnOpenMerge");
  config.mergeOutput = OutputMergeMode::MERGE;
  config.outputCompress = false;
  config.outputKeepFiles = true;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  size_t parts = 4;
  osm2rdf::util::Output o{config, output, parts};
  o.open();
  // Write content
  o.write("a", 0);
  o.write("b", 1);
  o.write("c", 2);
  o.write("d", 3);
  o.flush();
  o.close("p", "s");

  // Check content
  {
    std::stringstream resultBuffer;
    std::ifstream resultFile(output);
    ASSERT_TRUE(resultFile.is_open());
    ASSERT_TRUE(resultFile.good());
    resultBuffer << resultFile.rdbuf();
    ASSERT_EQ("pabcds", resultBuffer.str());
    resultFile.close();
    resultBuffer.clear();
  }

  // Reopen file for writing -> this clears the file.
  o.open();
  {
    std::stringstream resultBuffer;
    std::ifstream resultFile(output);
    ASSERT_TRUE(resultFile.is_open());
    ASSERT_TRUE(resultFile.good());
    resultBuffer << resultFile.rdbuf();
    ASSERT_EQ("", resultBuffer.str());
    resultFile.close();
    resultBuffer.clear();
  }

  // Write new content
  o.write("z", 3);
  o.write("y", 2);
  o.write("x", 1);
  o.write("w", 0);
  o.flush();
  o.close("p", "s");

  // Check new content
  {
    std::stringstream resultBuffer;
    std::ifstream resultFile(output);
    ASSERT_TRUE(resultFile.is_open());
    ASSERT_TRUE(resultFile.good());
    resultBuffer << resultFile.rdbuf();
    ASSERT_EQ("pwxyzs", resultBuffer.str());
    resultFile.close();
    resultBuffer.clear();
  }

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));
}

}  // namespace osm2rdf::util