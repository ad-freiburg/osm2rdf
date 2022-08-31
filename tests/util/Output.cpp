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

#include "osm2rdf/util/Output.h"

#include <iostream>

#include "gtest/gtest.h"

namespace osm2rdf::util {

// ____________________________________________________________________________
size_t countFilesInPath(const std::filesystem::path path) {
  return std::distance(std::filesystem::directory_iterator(path),
                       std::filesystem::directory_iterator());
}

// ____________________________________________________________________________
TEST(UTIL_Output, partFilenameSingleDigit) {
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_UTIL_Output", "partFilenameSingleDigit");
  osm2rdf::util::Output o{config, "test", 4};
  // Normal parts
  ASSERT_EQ("test.part_1", o.partFilename(0));
  ASSERT_EQ("test.part_2", o.partFilename(1));
  ASSERT_EQ("test.part_3", o.partFilename(2));
  ASSERT_EQ("test.part_4", o.partFilename(3));

  // Handle prefix (-1) and suffix (-2) parts
  ASSERT_EQ("test.part_0", o.partFilename(-1));
  ASSERT_EQ("test.part_5", o.partFilename(-2));
}

// ____________________________________________________________________________
TEST(UTIL_Output, partFilenameMultipleDigits) {
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_UTIL_Output", "partFilenameMultipleDigits");
  osm2rdf::util::Output o{config, "test", 16};
  // Normal parts
  ASSERT_EQ("test.part_01", o.partFilename(0));
  ASSERT_EQ("test.part_02", o.partFilename(1));
  ASSERT_EQ("test.part_03", o.partFilename(2));
  ASSERT_EQ("test.part_04", o.partFilename(3));
  ASSERT_EQ("test.part_05", o.partFilename(4));
  ASSERT_EQ("test.part_06", o.partFilename(5));
  ASSERT_EQ("test.part_07", o.partFilename(6));
  ASSERT_EQ("test.part_08", o.partFilename(7));
  ASSERT_EQ("test.part_09", o.partFilename(8));
  ASSERT_EQ("test.part_10", o.partFilename(9));
  ASSERT_EQ("test.part_11", o.partFilename(10));
  ASSERT_EQ("test.part_12", o.partFilename(11));
  ASSERT_EQ("test.part_13", o.partFilename(12));
  ASSERT_EQ("test.part_14", o.partFilename(13));
  ASSERT_EQ("test.part_15", o.partFilename(14));
  ASSERT_EQ("test.part_16", o.partFilename(15));

  // Handle prefix (-1) and suffix (-2) parts
  ASSERT_EQ("test.part_00", o.partFilename(-1));
  ASSERT_EQ("test.part_17", o.partFilename(-2));
}

// ____________________________________________________________________________
TEST(UTIL_Output, WriteIntoCurrentPartFile) {
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_UTIL_Output", "WriteIntoCurrentPartFile");
  config.mergeOutput = OutputMergeMode::NONE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  size_t parts = 4;
  osm2rdf::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
  // 4 parts + prefix + suffix
  ASSERT_EQ(parts + 2, countFilesInPath(config.output));
  o.write("a");
  o.write("b");
  o.write("c");
  o.write("d");
  o.flush();
  o.close();

  // All data should be written by thread with id 0
  for (size_t i = 1; i < parts; ++i) {
    ASSERT_GT(std::filesystem::file_size(o.partFilename(0)),
              std::filesystem::file_size(o.partFilename(i)));
  }

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));
}

// ____________________________________________________________________________
TEST(UTIL_Output, WriteIntoCurrentPartStdOut) {
  // Capture std::cout
  std::stringstream buffer;
  std::streambuf* sbuf = std::cout.rdbuf();
  std::cout.rdbuf(buffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.outputCompress = false;
  config.mergeOutput = OutputMergeMode::NONE;

  size_t parts = 4;
  osm2rdf::util::Output o{config, "", parts};
  o.open();
  o.write("a");
  o.write("b");
  o.write("c");
  o.write("d");
  o.flush();
  o.close();

  ASSERT_EQ("abcd", buffer.str());

  // Restore std::cout
  std::cout.rdbuf(sbuf);
}

// ____________________________________________________________________________
TEST(UTIL_OutputMergeMode, NONE) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_UTIL_OutputMergeMode", "NONE");
  config.mergeOutput = OutputMergeMode::NONE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  size_t parts = 4;
  osm2rdf::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
  // 4 parts + prefix + suffix
  ASSERT_EQ(parts + 2, countFilesInPath(config.output));
  o.write("a", 0);
  o.write("b", 1);
  o.write("c", 2);
  o.write("d", 3);
  o.flush();
  o.close();
  ASSERT_EQ(6, countFilesInPath(config.output));

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));
}

// ____________________________________________________________________________
TEST(UTIL_OutputMergeMode, CONCATENATE) {
  osm2rdf::config::Config config;
  config.output =
      config.getTempPath("TEST_UTIL_OutputMergeMode", "CONCATENATE");
  config.mergeOutput = OutputMergeMode::CONCATENATE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  size_t parts = 4;
  osm2rdf::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
  // 4 parts + prefix + suffix + final file
  ASSERT_EQ(parts + 3, countFilesInPath(config.output));
  o.write("a", 0);
  o.write("b", 1);
  o.write("c", 2);
  o.write("d", 3);
  o.flush();
  o.close();
  ASSERT_EQ(1, countFilesInPath(config.output));

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));
}

// ____________________________________________________________________________
TEST(UTIL_OutputMergeMode, MERGE) {
  osm2rdf::config::Config config;
  config.output = config.getTempPath("TEST_UTIL_OutputMergeMode", "MERGE");
  config.mergeOutput = OutputMergeMode::MERGE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output{config.output};
  output /= "file";

  size_t parts = 4;
  osm2rdf::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
  // 4 parts + prefix + suffix + final file
  ASSERT_EQ(parts + 3, countFilesInPath(config.output));
  o.write("a", 0);
  o.write("b", 1);
  o.write("c", 2);
  o.write("d", 3);
  o.flush();
  o.close();
  ASSERT_EQ(1, countFilesInPath(config.output));

  std::filesystem::remove_all(config.output);
  ASSERT_FALSE(std::filesystem::exists(config.output));
}

}  // namespace osm2rdf::util