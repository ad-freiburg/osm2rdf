// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2ttl/config/Config.h"

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2ttl/config/Constants.h"
#include "osm2ttl/config/ExitCode.h"
#include "osm2ttl/util/CacheFile.h"

namespace osm2ttl::config {

// ____________________________________________________________________________
void assertDefaultConfig(const osm2ttl::config::Config& config) {
  ASSERT_FALSE(config.noFacts);
  ASSERT_FALSE(config.noGeometricRelations);
  ASSERT_FALSE(config.storeLocationsOnDisk);

  ASSERT_FALSE(config.noAreaFacts);
  ASSERT_FALSE(config.noNodeFacts);
  ASSERT_FALSE(config.noRelationFacts);
  ASSERT_FALSE(config.noWayFacts);
  ASSERT_FALSE(config.noAreaGeometricRelations);
  ASSERT_FALSE(config.noNodeGeometricRelations);
  ASSERT_FALSE(config.noWayGeometricRelations);

  ASSERT_FALSE(config.addAreaEnvelope);
  ASSERT_FALSE(config.addInverseRelationDirection);
  ASSERT_TRUE(config.addSortMetadata);
  ASSERT_FALSE(config.addWayEnvelope);
  ASSERT_FALSE(config.addWayNodeOrder);
  ASSERT_FALSE(config.addWayMetadata);
  ASSERT_FALSE(config.adminRelationsOnly);
  ASSERT_FALSE(config.skipWikiLinks);

  ASSERT_EQ("osmadd", config.osm2ttlPrefix);

  ASSERT_FALSE(config.writeDAGDotFiles);

  ASSERT_FALSE(config.writeGeometricRelationStatistics);

  ASSERT_EQ(250, config.wktSimplify);
  ASSERT_EQ(5, config.wktDeviation);
  ASSERT_EQ(7, config.wktPrecision);

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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
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

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoHasSections) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::StartsWith(osm2ttl::config::constants::HEADER));
  ASSERT_THAT(res,
              ::testing::HasSubstr(osm2ttl::config::constants::SECTION_IO));
  ASSERT_THAT(res,
              ::testing::HasSubstr(osm2ttl::config::constants::SECTION_FACTS));
  ASSERT_THAT(
      res, ::testing::HasSubstr(osm2ttl::config::constants::SECTION_CONTAINS));
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::SECTION_MISCELLANEOUS));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoDumpPrefix) {
  const std::string prefix{"xxx"};

  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  std::stringstream res(config.getInfo(prefix));
  std::string line;

  while (std::getline(res, line, '\n')) {
    ASSERT_THAT(line, ::testing::StartsWith(prefix));
  }
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoFacts) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(osm2ttl::config::constants::NO_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoAreaDump) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noAreaFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::NO_AREA_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoNodeDump) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noNodeFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::NO_NODE_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoRelationDump) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noRelationFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::NO_RELATION_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoWayDump) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noWayFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res, ::testing::HasSubstr(osm2ttl::config::constants::NO_WAY_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddAreaEnvelope) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.addAreaEnvelope = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::ADD_AREA_ENVELOPE_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayEnvelope) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.addWayEnvelope = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::ADD_WAY_ENVELOPE_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayMetadata) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.addWayMetadata = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::ADD_WAY_METADATA_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayNodeOrder) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.addWayNodeOrder = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::ADD_WAY_NODE_ORDER_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoGeometricRelations) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::NO_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoAreaGeomRelations) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noAreaGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2ttl::config::constants::NO_AREA_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoNodeGeomRelations) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noNodeGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2ttl::config::constants::NO_NODE_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoWayGeomRelations) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.noWayGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::NO_WAY_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddInverseRelationDirection) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.addInverseRelationDirection = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res,
      ::testing::HasSubstr(
          osm2ttl::config::constants::ADD_INVERSE_RELATION_DIRECTION_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAdminRelationsOnly) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.adminRelationsOnly = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::ADMIN_RELATIONS_ONLY_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoWriteDAGDotFiles) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.writeDAGDotFiles = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::WRITE_DAG_DOT_FILES_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoWriteGeometricRelationStatistics) {
  osm2ttl::config::Config config;
  assertDefaultConfig(config);
  config.writeGeometricRelationStatistics = true;

  const std::string res = config.getInfo("");

#ifdef ENABLE_GEOMETRY_STATISTIC
  ASSERT_THAT(
      res,
      ::testing::HasSubstr(
          osm2ttl::config::constants::WRITE_GEOM_RELATION_STATISTICS_INFO));
#else
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2ttl::config::constants::
                           WRITE_GEOM_RELATION_STATISTICS_INFO_DISABLED));
#endif
}

}  // namespace osm2ttl::config