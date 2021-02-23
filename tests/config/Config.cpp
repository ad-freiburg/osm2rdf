// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/config/Config.h"

#include "gtest/gtest.h"
#include "osm2ttl/config/ExitCode.h"
#include "osm2ttl/util/CacheFile.h"

namespace osm2ttl::config {

// ____________________________________________________________________________
void assertDefaultConfig(const osm2ttl::config::Config& config) {
  ASSERT_FALSE(config.noDump);
  ASSERT_FALSE(config.noContains);
  ASSERT_FALSE(config.storeLocationsOnDisk);

  ASSERT_FALSE(config.noAreaDump);
  ASSERT_FALSE(config.noNodeDump);
  ASSERT_FALSE(config.noRelationDump);
  ASSERT_FALSE(config.noWayDump);

  ASSERT_FALSE(config.addAreaSources);
  ASSERT_FALSE(config.addEnvelope);
  ASSERT_FALSE(config.addMemberNodes);
  ASSERT_FALSE(config.adminRelationsOnly);
  ASSERT_FALSE(config.expandedData);
  ASSERT_FALSE(config.metaData);
  ASSERT_FALSE(config.skipWikiLinks);
  ASSERT_FALSE(config.addInverseRelationDirection);

  ASSERT_EQ("osmadd", config.osm2ttlPrefix);

  ASSERT_FALSE(config.writeDotFiles);

  ASSERT_FALSE(config.writeStatistics);

  ASSERT_EQ(250, config.wktSimplify);
  ASSERT_EQ(5, config.wktDeviation);
  ASSERT_EQ(12, config.wktPrecision);

  ASSERT_EQ(osm2ttl::util::OutputMergeMode::CONCATENATE, config.mergeOutput);
  ASSERT_TRUE(config.outputCompress);

  ASSERT_EQ(std::filesystem::temp_directory_path(), config.cache);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getTempPath) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  ASSERT_EQ("/tmp/prefix-suffix", config.getTempPath("prefix", "suffix"));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpShort) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("-h")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2ttl::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

TEST(CONFIG_Config, fromArgsHelpLong) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("--help")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2ttl::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

TEST(CONFIG_Config, fromArgsHelpAdvanced) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("-h"),
                      const_cast<char*>("-h")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2ttl::config::ExitCode::SUCCESS),
              "--add-inverse-relation-direction");
}

TEST(CONFIG_Config, fromArgsHelpExpert) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("-h"),
                      const_cast<char*>("-h"), const_cast<char*>("-h")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2ttl::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputCompressAddExtension) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  osm2ttl::util::CacheFile dummyInput("/tmp/dummyInput");

  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("-o"),
                      const_cast<char*>("/tmp/output"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("/tmp/output.bz2", config.output.string());
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputCompressKeepExtension) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  osm2ttl::util::CacheFile dummyInput("/tmp/dummyInput");

  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("-o"),
                      const_cast<char*>("/tmp/output.bz2"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("/tmp/output.bz2", config.output.string());
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputCompressIgnoreExtensionOnStdout) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  osm2ttl::util::CacheFile dummyInput("/tmp/dummyInput");

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsCacheNotFoundShort) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("-t"),
                      const_cast<char*>("/i/do/not/exist")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2ttl::config::ExitCode::CACHE_NOT_EXISTS),
      "^Cache location does not exist: \".+\"");
}
TEST(CONFIG_Config, fromArgsCacheNotFoundLong) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("--cache"),
                      const_cast<char*>("/i/do/not/exist")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2ttl::config::ExitCode::CACHE_NOT_EXISTS),
      "^Cache location does not exist: \".+\"");
}

TEST(CONFIG_Config, fromArgsCacheIsNotDirectory) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  osm2ttl::util::CacheFile cf("/tmp/dummy");
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>("--cache"),
                      const_cast<char*>("/tmp/dummy")};
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2ttl::config::ExitCode::CACHE_NOT_DIRECTORY),
      "^Cache location not a directory: \".+\"");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsEmpty) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 1;
  char* argv[argc] = {const_cast<char*>("")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2ttl::config::ExitCode::INPUT_MISSING),
      "^No input specified!");
}

TEST(CONFIG_Config, fromArgsInputNotFound) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""),
                      const_cast<char*>("/i/do/not/exist")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2ttl::config::ExitCode::INPUT_NOT_EXISTS),
      "^Input does not exist: \".+\"");
}

TEST(CONFIG_Config, fromArgsInputIsDirectory) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""),
                      const_cast<char*>(config.cache.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2ttl::config::ExitCode::INPUT_IS_DIRECTORY),
      "^Input is a directory: \".+\"");
}

}  // namespace osm2ttl::config