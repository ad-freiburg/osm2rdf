// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

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

#include "osm2rdf/config/Config.h"

#include "boost/version.hpp"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2rdf/config/Constants.h"
#include "osm2rdf/config/ExitCode.h"
#include "osm2rdf/util/CacheFile.h"

namespace osm2rdf::config {

// ____________________________________________________________________________
void assertDefaultConfig(const osm2rdf::config::Config& config) {
  ASSERT_FALSE(config.noFacts);
  ASSERT_FALSE(config.noGeometricRelations);
  ASSERT_TRUE(config.storeLocationsOnDisk.empty());

  ASSERT_FALSE(config.noAreaFacts);
  ASSERT_FALSE(config.noNodeFacts);
  ASSERT_FALSE(config.noRelationFacts);
  ASSERT_FALSE(config.noWayFacts);
  ASSERT_FALSE(config.noAreaGeometricRelations);
  ASSERT_FALSE(config.noNodeGeometricRelations);
  ASSERT_FALSE(config.noWayGeometricRelations);

  ASSERT_FALSE(config.addAreaConvexHull);
  ASSERT_FALSE(config.addAreaEnvelope);
  ASSERT_FALSE(config.addAreaEnvelopeRatio);
  ASSERT_FALSE(config.addAreaOrientedBoundingBox);
  ASSERT_FALSE(config.addNodeConvexHull);
  ASSERT_FALSE(config.addNodeEnvelope);
  ASSERT_FALSE(config.addNodeOrientedBoundingBox);
  ASSERT_FALSE(config.addRelationBorderMembers);
  ASSERT_FALSE(config.addRelationConvexHull);
  ASSERT_FALSE(config.addRelationEnvelope);
  ASSERT_FALSE(config.addRelationOrientedBoundingBox);
  ASSERT_TRUE(config.addSortMetadata);
  ASSERT_FALSE(config.addWayConvexHull);
  ASSERT_FALSE(config.addWayEnvelope);
  ASSERT_FALSE(config.addWayOrientedBoundingBox);
  ASSERT_FALSE(config.addWayNodeGeometry);
  ASSERT_FALSE(config.addWayNodeOrder);
  ASSERT_FALSE(config.addWayNodeSpatialMetadata);
  ASSERT_FALSE(config.addWayMetadata);
  ASSERT_FALSE(config.adminRelationsOnly);
  ASSERT_FALSE(config.skipWikiLinks);

  ASSERT_EQ(0, config.semicolonTagKeys.size());

  ASSERT_FALSE(config.writeDAGDotFiles);

  ASSERT_FALSE(config.writeRDFStatistics);

  ASSERT_EQ(0, config.simplifyGeometries);
  ASSERT_EQ(250, config.geometriesDumpMinNumPointsForSimplification);
  ASSERT_EQ(5, config.geometriesDumpDeviation);
  ASSERT_EQ(7, config.geometriesDumpPrecision);

  ASSERT_EQ(osm2rdf::util::OutputMergeMode::CONCATENATE, config.mergeOutput);
  ASSERT_TRUE(config.outputCompress);
  ASSERT_FALSE(config.outputKeepFiles);

  ASSERT_EQ(std::filesystem::temp_directory_path(), config.cache);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getTempPath) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  ASSERT_EQ("/tmp/prefix-suffix", config.getTempPath("prefix", "suffix"));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpShort) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "-" + osm2rdf::config::constants::HELP_OPTION_SHORT;
  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "--" + osm2rdf::config::constants::HELP_OPTION_LONG;
  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpAdvancedShortMultiple) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "-" + osm2rdf::config::constants::HELP_OPTION_SHORT;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              osm2rdf::config::constants::NO_GEOM_RELATIONS_OPTION_HELP);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpAdvancedShortCombined) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "-" + osm2rdf::config::constants::HELP_OPTION_SHORT +
                   osm2rdf::config::constants::HELP_OPTION_SHORT;
  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              osm2rdf::config::constants::NO_GEOM_RELATIONS_OPTION_HELP);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpAdvancedLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "--" + osm2rdf::config::constants::HELP_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              osm2rdf::config::constants::NO_GEOM_RELATIONS_OPTION_HELP);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpExpertShortMultiple) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "-" + osm2rdf::config::constants::HELP_OPTION_SHORT;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>(arg.c_str()),
                      const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpExpertShortCombined) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "-" + osm2rdf::config::constants::HELP_OPTION_SHORT +
                   osm2rdf::config::constants::HELP_OPTION_SHORT +
                   osm2rdf::config::constants::HELP_OPTION_SHORT;
  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsHelpExpertLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "--" + osm2rdf::config::constants::HELP_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>(arg.c_str()),
                      const_cast<char*>(arg.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::SUCCESS),
              "^Allowed options:");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputCompressAddExtension) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile dummyInput("/tmp/dummyInput");

  const auto arg = "-" + osm2rdf::config::constants::OUTPUT_OPTION_SHORT;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/output"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("/tmp/output.bz2", config.output.string());
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputCompressKeepExtension) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile dummyInput("/tmp/dummyInput");

  const auto arg = "-" + osm2rdf::config::constants::OUTPUT_OPTION_SHORT;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/output.bz2"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("/tmp/output.bz2", config.output.string());
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputCompressIgnoreExtensionOnStdout) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile dummyInput("/tmp/dummyInput");

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsCacheNotFoundShort) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "-" + osm2rdf::config::constants::CACHE_OPTION_SHORT;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/i/do/not/exist")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::CACHE_NOT_EXISTS),
      "^Cache location does not exist: \".+\"");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsCacheNotFoundLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const auto arg = "--" + osm2rdf::config::constants::CACHE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/i/do/not/exist")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::CACHE_NOT_EXISTS),
      "^Cache location does not exist: \".+\"");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsCacheIsNotDirectory) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummy");

  const auto arg = "--" + osm2rdf::config::constants::CACHE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummy")};
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::CACHE_NOT_DIRECTORY),
      "^Cache location not a directory: \".+\"");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsEmpty) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const int argc = 1;
  char* argv[argc] = {const_cast<char*>("")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::INPUT_MISSING),
      "^No input specified!");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsUnkonwOption) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile dummyInput("/tmp/dummyInput");

  const auto arg = "-" + osm2rdf::config::constants::OUTPUT_OPTION_SHORT;
  const int argc = 5;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/output"),
                      const_cast<char*>("--unknown-arg"),
                      const_cast<char*>("/tmp/dummyInput")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::UNKNOWN_ARGUMENT),
      "^Unknown argument");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsInvalidValue) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile dummyInput("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::GEOMETRIES_DUMP_DEVIATION_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/output"),
                      const_cast<char*>("/tmp/dummyInput")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(config.fromArgs(argc, argv),
              ::testing::ExitedWithCode(osm2rdf::config::ExitCode::FAILURE),
              "^Invalid Option");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsInputNotFound) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""),
                      const_cast<char*>("/i/do/not/exist")};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::INPUT_NOT_EXISTS),
      "^Input does not exist: \".+\"");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsInputIsDirectory) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const int argc = 2;
  char* argv[argc] = {const_cast<char*>(""),
                      const_cast<char*>(config.cache.c_str())};
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ASSERT_EXIT(
      config.fromArgs(argc, argv),
      ::testing::ExitedWithCode(osm2rdf::config::ExitCode::INPUT_IS_DIRECTORY),
      "^Input is a directory: \".+\"");
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoFactsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_FACTS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noFacts);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoGeometricRelationsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::NO_GEOM_RELATIONS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsStoreLocationsOnDiskLongImplicit) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::STORE_LOCATIONS_ON_DISK_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ("sparse", config.storeLocationsOnDisk);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsStoreLocationsOnDiskLongSparse) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" +
                   osm2rdf::config::constants::STORE_LOCATIONS_ON_DISK_LONG +
                   "=sparse";
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ("sparse", config.storeLocationsOnDisk);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsStoreLocationsOnDiskLongDense) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" +
                   osm2rdf::config::constants::STORE_LOCATIONS_ON_DISK_LONG +
                   "=dense";
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ("dense", config.storeLocationsOnDisk);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoAreasLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_AREA_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noAreaFacts);
  ASSERT_TRUE(config.noAreaGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoNodesLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_NODE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noNodeFacts);
  ASSERT_TRUE(config.noNodeGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoRelationsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_RELATION_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noRelationFacts);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoWaysLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_WAY_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noWayFacts);
  ASSERT_TRUE(config.noWayGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoAreaFactsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_AREA_FACTS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noAreaFacts);
  ASSERT_FALSE(config.noAreaGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoNodeFactsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_NODE_FACTS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noNodeFacts);
  ASSERT_FALSE(config.noNodeGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoRelationFactsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::NO_RELATION_FACTS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noRelationFacts);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoWayFactsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::NO_WAY_FACTS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.noWayFacts);
  ASSERT_FALSE(config.noWayGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoAreaGeometricRelationsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::NO_AREA_GEOM_RELATIONS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_FALSE(config.noAreaFacts);
  ASSERT_TRUE(config.noAreaGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoNodeGeometricRelationsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::NO_NODE_GEOM_RELATIONS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_FALSE(config.noNodeFacts);
  ASSERT_TRUE(config.noNodeGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsNoWayGeometricRelationsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::NO_WAY_GEOM_RELATIONS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_FALSE(config.noWayFacts);
  ASSERT_TRUE(config.noWayGeometricRelations);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddAreaConvexHullLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_AREA_CONVEX_HULL_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addAreaConvexHull);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddAreaEnvelopeLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_AREA_ENVELOPE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addAreaEnvelope);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddAreaEnvelopeRatioLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_AREA_ENVELOPE_RATIO_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addAreaEnvelopeRatio);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddAreaOrientedBoundingBoxLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_AREA_ORIENTED_BOUNDING_BOX_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addAreaOrientedBoundingBox);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddNodeConvexHullLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_NODE_CONVEX_HULL_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addNodeConvexHull);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddNodeEnvelopeLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_NODE_ENVELOPE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addNodeEnvelope);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddNodeOrientedBoundingBoxLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_NODE_ORIENTED_BOUNDING_BOX_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addNodeOrientedBoundingBox);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddRelationBorderMembersLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_RELATION_BORDER_MEMBERS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addRelationBorderMembers);
}

#if BOOST_VERSION >= 107800
// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddRelationConvexHullLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_RELATION_CONVEX_HULL_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addRelationConvexHull);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddRelationEnvelopeLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_RELATION_ENVELOPE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addRelationEnvelope);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddRelationOrientedBoundingBoxLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_RELATION_ORIENTED_BOUNDING_BOX_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addRelationOrientedBoundingBox);
}
#endif  // BOOST_VERSION >= 107800

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayConvexHullLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_WAY_CONVEX_HULL_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayConvexHull);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayEnvelopeLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_WAY_ENVELOPE_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayEnvelope);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayOrientedBoundingBOxLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_WAY_ORIENTED_BOUNDING_BOX_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayOrientedBoundingBox);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayMetadataLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_WAY_METADATA_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayMetadata);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayNodeGeomentryLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  osm2rdf::util::CacheFile cf("/tmp/dummyInput");
  const auto arg =
      "--" + osm2rdf::config::constants::ADD_WAY_NODE_GEOMETRY_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayNodeGeometry);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayNodeOrderLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADD_WAY_NODE_ORDER_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayNodeOrder);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAddWayNodeSpatialMetadataLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  osm2rdf::util::CacheFile cf("/tmp/dummyInput");
  const auto arg =
      "--" +
      osm2rdf::config::constants::ADD_WAY_NODE_SPATIAL_METADATA_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.addWayNodeSpatialMetadata);
  ASSERT_TRUE(config.addWayNodeOrder);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsAdminRelationsOnlyLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::ADMIN_RELATIONS_ONLY_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.adminRelationsOnly);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsSkipWikiLinksLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::SKIP_WIKI_LINKS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.skipWikiLinks);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsSimplifyGeometriesLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("25"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ(25, config.simplifyGeometries);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsSimplifyGeometriesDumpLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_DUMP_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("25"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ(25, config.geometriesDumpMinNumPointsForSimplification);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsGeometriesDumpDeviationLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::GEOMETRIES_DUMP_DEVIATION_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("25"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ(25, config.geometriesDumpDeviation);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsGeometriesDumpPrecisionLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg = "--" + osm2rdf::config::constants::GEOMETRIES_DUMP_PRECISION_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("2"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ(2, config.geometriesDumpPrecision);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsSemicolonTagKeysSingleLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::SEMICOLON_TAG_KEYS_OPTION_LONG;
  const int argc = 4;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("ref"),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ(1, config.semicolonTagKeys.size());
  ASSERT_EQ(1, config.semicolonTagKeys.count("ref"));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsSemicolonTagKeysMultipleLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::SEMICOLON_TAG_KEYS_OPTION_LONG;
  const int argc = 8;
  char* argv[argc] = {
      const_cast<char*>(""),     const_cast<char*>(arg.c_str()),
      const_cast<char*>("ref"),  const_cast<char*>(arg.c_str()),
      const_cast<char*>("ref2"), const_cast<char*>(arg.c_str()),
      const_cast<char*>("ref"),  const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_EQ(2, config.semicolonTagKeys.size());
  ASSERT_EQ(1, config.semicolonTagKeys.count("ref"));
  ASSERT_EQ(1, config.semicolonTagKeys.count("ref2"));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsWriteRDFRelationStatisticsLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::WRITE_RDF_STATISTICS_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.writeRDFStatistics);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, fromArgsOutputKeepFilesLong) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  osm2rdf::util::CacheFile cf("/tmp/dummyInput");

  const auto arg =
      "--" + osm2rdf::config::constants::OUTPUT_KEEP_FILES_OPTION_LONG;
  const int argc = 3;
  char* argv[argc] = {const_cast<char*>(""), const_cast<char*>(arg.c_str()),
                      const_cast<char*>("/tmp/dummyInput")};
  config.fromArgs(argc, argv);
  ASSERT_EQ("", config.output.string());
  ASSERT_TRUE(config.outputKeepFiles);
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoHasSections) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::StartsWith(osm2rdf::config::constants::HEADER));
  ASSERT_THAT(res,
              ::testing::HasSubstr(osm2rdf::config::constants::SECTION_IO));
  ASSERT_THAT(res,
              ::testing::HasSubstr(osm2rdf::config::constants::SECTION_FACTS));
  ASSERT_THAT(
      res, ::testing::HasSubstr(osm2rdf::config::constants::SECTION_CONTAINS));
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::SECTION_MISCELLANEOUS));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoDumpPrefix) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);

  const std::string prefix{"xxx"};
  std::stringstream res(config.getInfo(prefix));
  std::string line;

  while (std::getline(res, line, '\n')) {
    ASSERT_THAT(line, ::testing::StartsWith(prefix));
  }
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoFacts) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(osm2rdf::config::constants::NO_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoAreaDump) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noAreaFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::NO_AREA_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoNodeDump) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noNodeFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::NO_NODE_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoRelationDump) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noRelationFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::NO_RELATION_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoWayDump) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noWayFacts = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res, ::testing::HasSubstr(osm2rdf::config::constants::NO_WAY_FACTS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddAreaConvexHull) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addAreaConvexHull = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_AREA_CONVEX_HULL_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddAreaEnvelope) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addAreaEnvelope = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_AREA_ENVELOPE_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddAreaOrientedBoundingBox) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addAreaOrientedBoundingBox = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res,
      ::testing::HasSubstr(
          osm2rdf::config::constants::ADD_AREA_ORIENTED_BOUNDING_BOX_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddAreaEnvelopeRatio) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addAreaEnvelopeRatio = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2rdf::config::constants::ADD_AREA_ENVELOPE_RATIO_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddNodeConvexHull) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addNodeConvexHull = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_NODE_CONVEX_HULL_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddNodeEnvelope) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addNodeEnvelope = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_NODE_ENVELOPE_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddNodeOrientedBoundingBox) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addNodeOrientedBoundingBox = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res,
      ::testing::HasSubstr(
          osm2rdf::config::constants::ADD_NODE_ORIENTED_BOUNDING_BOX_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddRelationBorderMembers) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addRelationBorderMembers = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res, ::testing::HasSubstr(
               osm2rdf::config::constants::ADD_RELATION_BORDER_MEMBERS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddRelationConvexHull) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addRelationConvexHull = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2rdf::config::constants::ADD_RELATION_CONVEX_HULL_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddRelationEnvelope) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addRelationEnvelope = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_RELATION_ENVELOPE_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddRelationOrientedBoundingBox) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addRelationOrientedBoundingBox = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res,
      ::testing::HasSubstr(
          osm2rdf::config::constants::ADD_RELATION_ORIENTED_BOUNDING_BOX_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayConvexHull) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayConvexHull = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_WAY_CONVEX_HULL_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayEnvelope) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayEnvelope = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_WAY_ENVELOPE_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayOrientedBoundingBox) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayOrientedBoundingBox = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res, ::testing::HasSubstr(
               osm2rdf::config::constants::ADD_WAY_ORIENTED_BOUNDING_BOX_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayMetadata) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayMetadata = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_WAY_METADATA_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayNodeGeometry) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayNodeGeometry = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_WAY_NODE_GEOMETRY_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayNodeOrder) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayNodeOrder = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADD_WAY_NODE_ORDER_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAddWayNodeSpatialMetadata) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.addWayNodeSpatialMetadata = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res, ::testing::HasSubstr(
               osm2rdf::config::constants::ADD_WAY_NODE_SPATIAL_METADATA_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoSemicolonTagKeys) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.semicolonTagKeys.insert("ref");

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::SEMICOLON_TAG_KEYS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoSimplifyGeometries) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.simplifyGeometries = 250;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoSimplifyGeometriesDump) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.geometriesDumpMinNumPointsForSimplification = 250;

  const std::string res = config.getInfo("");
  ASSERT_THAT(
      res, ::testing::HasSubstr(osm2rdf::config::constants::SIMPLIFY_GEOMETRIES_DUMP_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoSkipWikiLinks) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.skipWikiLinks = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::SKIP_WIKI_LINKS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoGeometricRelations) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::NO_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoAreaGeomRelations) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noAreaGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2rdf::config::constants::NO_AREA_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoNoNodeGeomRelations) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noNodeGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2rdf::config::constants::NO_NODE_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoWayGeomRelations) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.noWayGeometricRelations = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::NO_WAY_GEOM_RELATIONS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoAdminRelationsOnly) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.adminRelationsOnly = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::ADMIN_RELATIONS_ONLY_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoWriteDAGDotFiles) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.writeDAGDotFiles = true;

  const std::string res = config.getInfo("");
  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::WRITE_DAG_DOT_FILES_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoWriteRDFStatistics) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.writeRDFStatistics = true;

  const std::string res = config.getInfo("");

  ASSERT_THAT(res, ::testing::HasSubstr(
                       osm2rdf::config::constants::WRITE_RDF_STATISTICS_INFO));
}

// ____________________________________________________________________________
TEST(CONFIG_Config, getInfoOutputKeepFiles) {
  osm2rdf::config::Config config;
  assertDefaultConfig(config);
  config.outputKeepFiles = true;

  const std::string res = config.getInfo("");

  ASSERT_THAT(res,
              ::testing::HasSubstr(
                  osm2rdf::config::constants::OUTPUT_KEEP_FILES_OPTION_INFO));
}

}  // namespace osm2rdf::config
