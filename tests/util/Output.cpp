// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/util/Output.h"

#include <iostream>

#include "gtest/gtest.h"

namespace osm2ttl::util {

size_t countFilesInPath(const std::filesystem::path path) {
  return std::distance(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator());
}

TEST(Output, partFilename) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  osm2ttl::util::Output o{config, "test", 4};
  // Normal parts
  ASSERT_EQ("test.part_1", o.partFilename(0));
  ASSERT_EQ("test.part_2", o.partFilename(1));
  ASSERT_EQ("test.part_3", o.partFilename(2));
  ASSERT_EQ("test.part_4", o.partFilename(3));

  // Handle prefix (-1) and suffix (-2) parts
  ASSERT_EQ("test.part_0", o.partFilename(-1));
  ASSERT_EQ("test.part_5", o.partFilename(-2));
}

TEST(OutputMergeMode, NONE) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("OutputMergeMode", "NONE");
  config.mergeOutput = OutputMergeMode::NONE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output {config.output};
  output /= "file";

  size_t parts = 4;
  osm2ttl::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
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

TEST(OutputMergeMode, CONCATENATE) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("OutputMergeMode", "CONCATENATE");
  config.mergeOutput = OutputMergeMode::CONCATENATE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output {config.output};
  output /= "file";

  size_t parts = 4;
  osm2ttl::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
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

TEST(OutputMergeMode, MERGE) {
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.output = config.getTempPath("OutputMergeMode", "MERGE");
  config.mergeOutput = OutputMergeMode::MERGE;
  ASSERT_FALSE(std::filesystem::exists(config.output));
  std::filesystem::create_directories(config.output);
  ASSERT_TRUE(std::filesystem::exists(config.output));
  ASSERT_TRUE(std::filesystem::is_directory(config.output));
  std::filesystem::path output {config.output};
  output /= "file";

  size_t parts = 4;
  osm2ttl::util::Output o{config, output, parts};
  ASSERT_EQ(0, countFilesInPath(config.output));
  o.open();
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

}