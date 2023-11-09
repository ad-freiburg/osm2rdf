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

#include <filesystem>
#include <string>
#include <vector>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/OsmiumHandler.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/Output.h"

std::vector<std::filesystem::path> getFilesWithPrefixFromPath(
    std::filesystem::path path, std::string_view prefix) {
  std::vector<std::filesystem::path> result;
  for (auto& e : std::filesystem::directory_iterator(path)) {
    if (!e.is_regular_file()) {
      continue;
    }
    if (e.path().filename().string().find(prefix, 0) != 0) {
      continue;
    }
    result.push_back(e.path());
  }
  return result;
}

// ____________________________________________________________________________
TEST(E2E, singleNode) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "singleNode.osm");
  std::ofstream inputFile(config.input);
  // Copied from
  // https://wiki.openstreetmap.org/w/index.php?title=OSM_XML&oldid=2081001
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n"
            << "<node id=\"298884269\" lat=\"54.0901746\" lon=\"12.2482632\" "
               "user=\"SvenHRO\" uid=\"46882\" visible=\"true\" version=\"1\" "
               "changeset=\"676636\" timestamp=\"2008-09-21T21:37:45Z\"/>"
            << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:1 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:0 dumped: 0 geometry: 0\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "@prefix wd: <http://www.wikidata.org/entity/> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, singleNodeWithTags) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "singleNodeWithTags.osm");
  std::ofstream inputFile(config.input);
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  // ... https://www.openstreetmap.org/node/240092010
  inputFile << "<node id=\"240092010\" lat=\"47.9960901\" lon=\"7.8494005\" "
               "visible=\"true\" version=\"1\">"
               "  <tag k=\"alt_name\" v=\"Freiburg i. Br.\"/>\n"
               "  <tag k=\"name\" v=\"Freiburg im Breisgau\"/>\n"
               "  <tag k=\"name:ja\" v=\"フライブルク\"/>\n"
               "  <tag k=\"short_name\" v=\"Freiburg\"/>\n"
               "  <tag k=\"wikidata\" v=\"Q2833\"/>\n"
               "  <tag k=\"wikipedia\" v=\"de:Freiburg im Breisgau\"/>\n"
               "</node>\n";
  inputFile << "<node id=\"925950614\" lat=\"47.9878947\" lon=\"7.8704212\" "
               "visible=\"true\" version=\"1\">"
               "  <tag k=\"brand:wikidata\" v=\"Q41171672\"/>\n"
               "  <tag k=\"brand:wikipedia\" v=\"en:Aldi\"/>\n"
               "</node>\n";
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:2 dumped: 2 geometry: 2\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:0 dumped: 0 geometry: 0\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmnode:240092010 rdf:type osm:node .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmnode:925950614 rdf:type osm:node .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmnode:240092010 geo:hasGeometry \"POINT(7.8494005 "
                  "47.9960901)\"^^geo:wktLiteral .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmnode:240092010 osmkey:alt_name \"Freiburg i. Br.\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmnode:240092010 osmkey:name \"Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmnode:240092010 osmkey:name:ja "
                                   "\"\xE3\x83\x95\xE3\x83\xA9\xE3\x82\xA4\xE3"
                                   "\x83\x96\xE3\x83\xAB\xE3\x82\xAF\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmnode:240092010 osmkey:short_name \"Freiburg\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmnode:240092010 osmkey:wikidata \"Q2833\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmnode:240092010 osm:wikidata wd:Q2833 .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmnode:240092010 osmkey:wikipedia "
                                   "\"de:Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmnode:240092010 osm:wikipedia "
          "<https://de.wikipedia.org/wiki/Freiburg%20im%20Breisgau> .\n"));

  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmnode:925950614 osmkey:brand:wikidata \"Q41171672\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmnode:925950614 osm:brand:wikidata wd:Q41171672 .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmnode:925950614 osmkey:brand:wikipedia "
                                   "\"en:Aldi\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmnode:925950614 osm:brand:wikipedia "
                                   "<https://en.wikipedia.org/wiki/Aldi> .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, singleWayWithTagsAndNodes) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "singleWayWithTagsAndNodes.osm");
  std::ofstream inputFile(config.input);
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  // ... https://www.openstreetmap.org/node/1137213233 see e2e/w98284318.xml
  // for full file
  inputFile
      << "<way id=\"98284318\" visible=\"true\" version=\"10\">"
         "  <tag k=\"addr:city\" v=\"Freiburg im Breisgau\"/>\n"
         "  <tag k=\"addr:housenumber\" v=\"51\"/>\n"
         "  <tag k=\"addr:postcode\" v=\"79110\"/>\n"
         "  <tag k=\"addr:street\" v=\"Georges-Köhler-Allee\"/>\n"
         "  <tag k=\"building\" v=\"university\"/>\n"
         "  <tag k=\"building:levels\" v=\"4\"/>\n"
         "  <tag k=\"name\" v=\"Gebäude 51\"/>\n"
         "  <tag k=\"roof:levels\" v=\"1\"/>\n"
         "  <tag k=\"roof:shape\" v=\"hipped\"/>\n"
         "  <tag k=\"source:outline\" v=\"maps4bw (LGL, www.lgl-bw.de)\"/>\n"
         "</way>\n";
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 rdf:type osm:way .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:city \"Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:housenumber \"51\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:postcode \"79110\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:street \"Georges-Köhler-Allee\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:building \"university\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:building:levels \"4\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:name \"Gebäude 51\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:98284318 osmkey:roof:levels \"1\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:roof:shape \"hipped\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 osmkey:source:outline "
                                   "\"maps4bw (LGL, www.lgl-bw.de)\" .\n"));
  // No nodes -> no real geometry
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 geo:hasGeometry "
                                   "\"LINESTRING()\"^^geo:wktLiteral .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, osmWikiExample) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "osmWikiExample.osm");
  std::ofstream inputFile(config.input);
  // Based on
  // https://wiki.openstreetmap.org/w/index.php?title=OSM_XML&oldid=2081001
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  inputFile << " <node id=\"298884269\" lat=\"54.0901746\" lon=\"12.2482632\" "
               "visible=\"true\" version=\"1\"/>\n";
  inputFile << " <node id=\"261728686\" lat=\"54.0906309\" lon=\"12.2441924\" "
               "visible=\"true\" version=\"1\"/>\n";
  inputFile << " <node id=\"1831881213\" version=\"1\" lat=\"54.0900666\" "
               "lon=\"12.2539381\" visible=\"true\">\n"
               "  <tag k=\"name\" v=\"Neu Broderstorf\"/>\n"
               "  <tag k=\"traffic_sign\" v=\"city_limit\"/>\n"
               " </node>\n";
  inputFile << " <node id=\"298884272\" lat=\"54.0901447\" lon=\"12.2516513\" "
               "visible=\"true\" version=\"1\"/>\n";
  inputFile << " <way id=\"26659127\" visible=\"true\" version=\"5\">\n"
               "  <nd ref=\"298884269\"/>\n"
               "  <nd ref=\"261728686\"/>\n"
               "  <nd ref=\"298884272\"/>\n"
               "  <tag k=\"highway\" v=\"unclassified\"/>\n"
               "  <tag k=\"name\" v=\"Pastower Straße\"/>\n"
               " </way>\n";
  inputFile << " <relation id=\"56688\" visible=\"true\" version=\"28\">\n"
               "  <member type=\"node\" ref=\"298884269\" role=\"\"/>\n"
               "  <member type=\"node\" ref=\"261728686\" role=\"\"/>\n"
               "  <member type=\"way\" ref=\"26659127\" role=\"\"/>\n"
               "  <member type=\"node\" ref=\"1831881213\" role=\"\"/>\n"
               "  <tag k=\"name\" v=\"Küstenbus Linie 123\"/>\n"
               "  <tag k=\"network\" v=\"VVW\"/>\n"
               "  <tag k=\"operator\" v=\"Regionalverkehr Küste\"/>\n"
               "  <tag k=\"ref\" v=\"123\"/>\n"
               "  <tag k=\"route\" v=\"bus\"/>\n"
               "  <tag k=\"type\" v=\"route\"/>\n"
               " </relation>";
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:4 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:1 dumped: 1 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmnode:1831881213 osmkey:traffic_sign \"city_limit\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:26659127 osmkey:name \"Pastower Straße\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:26659127 geo:hasGeometry \"LINESTRING("));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmrel:56688 rdf:type osm:relation .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("_2 osm:id osmway:26659127 .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, building51NT) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.addAreaWayLinestrings = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "building51NT.osm");
  std::ofstream inputFile(config.input);

  std::vector<std::filesystem::path> nodes =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "n");
  std::vector<std::filesystem::path> ways =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "w");
  std::sort(nodes.begin(), nodes.end(),
            [](const auto& a, const auto& b) -> bool {
              return a.filename().string() < b.filename().string();
            });
  std::sort(ways.begin(), ways.end(), [](const auto& a, const auto& b) -> bool {
    return a.filename().string() < b.filename().string();
  });
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  for (const auto& f : nodes) {
    inputFile << std::ifstream(f).rdbuf();
  }
  for (const auto& f : ways) {
    inputFile << std::ifstream(f).rdbuf();
  }
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::NT> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:19 dumped: 3 geometry: 3\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(
      printedState,
      ::testing::HasSubstr("Contains relations for 3 nodes in 1 areas ...\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("<https://www.openstreetmap.org/way/98284318> "
                           "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type> "
                           "<https://www.openstreetmap.org/way> .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<https://www.openstreetmap.org/wiki/"
                               "Key:addr:city> \"Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<https://www.openstreetmap.org/wiki/"
                               "Key:addr:housenumber> \"51\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<https://www.openstreetmap.org/wiki/"
                               "Key:addr:postcode> \"79110\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("<https://www.openstreetmap.org/way/98284318> "
                           "<https://www.openstreetmap.org/wiki/"
                           "Key:addr:street> \"Georges-Köhler-Allee\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<https://www.openstreetmap.org/wiki/"
                               "Key:building> \"university\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<https://www.openstreetmap.org/wiki/"
                               "Key:building:levels> \"4\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "<https://www.openstreetmap.org/way/98284318> "
          "<https://www.openstreetmap.org/wiki/Key:name> \"Gebäude 51\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "<https://www.openstreetmap.org/way/98284318> "
          "<https://www.openstreetmap.org/wiki/Key:roof:levels> \"1\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<https://www.openstreetmap.org/wiki/"
                               "Key:roof:shape> \"hipped\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://www.openstreetmap.org/wiki/Key:source:outline> "
                  "\"maps4bw (LGL, www.lgl-bw.de)\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "<https://www.openstreetmap.org/way/98284318> "
          "<http://www.opengis.net/ont/geosparql#hasGeometry> \"LINESTRING(7"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "7)\"^^<http://www.opengis.net/ont/geosparql#wktLiteral> .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "<https://www.openstreetmap.org/way/98284318> "
                               "<http://www.opengis.net/ont/"
                               "geosparql#hasGeometry> \"MULTIPOLYGON(((7"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "0)))\"^^<http://www.opengis.net/ont/geosparql#wktLiteral> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://osm2rdf.cs.uni-freiburg.de/rdf#intersects_nonarea> "
                  "<https://www.openstreetmap.org/node/2110601105> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://osm2rdf.cs.uni-freiburg.de/rdf#contains_nonarea> "
                  "<https://www.openstreetmap.org/node/2110601105> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://osm2rdf.cs.uni-freiburg.de/rdf#intersects_nonarea> "
                  "<https://www.openstreetmap.org/node/2110601134> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://osm2rdf.cs.uni-freiburg.de/rdf#contains_nonarea> "
                  "<https://www.openstreetmap.org/node/2110601134> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://osm2rdf.cs.uni-freiburg.de/rdf#intersects_nonarea> "
                  "<https://www.openstreetmap.org/node/5190342871> .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "<https://www.openstreetmap.org/way/98284318> "
                  "<https://osm2rdf.cs.uni-freiburg.de/rdf#contains_nonarea> "
                  "<https://www.openstreetmap.org/node/5190342871> .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, building51TTL) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.addAreaWayLinestrings = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "building51TTL.osm");
  std::ofstream inputFile(config.input);

  std::vector<std::filesystem::path> nodes =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "n");
  std::vector<std::filesystem::path> ways =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "w");
  std::sort(nodes.begin(), nodes.end(),
            [](const auto& a, const auto& b) -> bool {
              return a.filename().string() < b.filename().string();
            });
  std::sort(ways.begin(), ways.end(), [](const auto& a, const auto& b) -> bool {
    return a.filename().string() < b.filename().string();
  });
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  for (const auto& f : nodes) {
    inputFile << std::ifstream(f).rdbuf();
  }
  for (const auto& f : ways) {
    inputFile << std::ifstream(f).rdbuf();
  }
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::TTL> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:19 dumped: 3 geometry: 3\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(
      printedState,
      ::testing::HasSubstr("Contains relations for 3 nodes in 1 areas ...\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 rdf:type osm:way .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:city \"Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:housenumber \"51\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:postcode \"79110\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:street \"Georges-Köhler-Allee\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:building \"university\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:building:levels \"4\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:name \"Gebäude 51\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:98284318 osmkey:roof:levels \"1\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:roof:shape \"hipped\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 osmkey:source:outline "
                                   "\"maps4bw (LGL, www.lgl-bw.de)\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 geo:hasGeometry \"LINESTRING(7"));
  ASSERT_THAT(printedData, ::testing::HasSubstr("7)\"^^geo:wktLiteral .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "smway:98284318 geo:hasGeometry \"MULTIPOLYGON(((7"));
  ASSERT_THAT(printedData, ::testing::HasSubstr("0)))\"^^geo:wktLiteral .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:2110601105 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:2110601105 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:2110601134 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:2110601134 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:5190342871 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:5190342871 .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, building51QLEVER) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.addAreaWayLinestrings = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "building51QLEVER.osm");
  std::ofstream inputFile(config.input);

  std::vector<std::filesystem::path> nodes =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "n");
  std::vector<std::filesystem::path> ways =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "w");
  std::sort(nodes.begin(), nodes.end(),
            [](const auto& a, const auto& b) -> bool {
              return a.filename().string() < b.filename().string();
            });
  std::sort(ways.begin(), ways.end(), [](const auto& a, const auto& b) -> bool {
    return a.filename().string() < b.filename().string();
  });
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  for (const auto& f : nodes) {
    inputFile << std::ifstream(f).rdbuf();
  }
  for (const auto& f : ways) {
    inputFile << std::ifstream(f).rdbuf();
  }
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:19 dumped: 3 geometry: 3\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(
      printedState,
      ::testing::HasSubstr("Contains relations for 3 nodes in 1 areas ...\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 rdf:type osm:way .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:city \"Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:housenumber \"51\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:postcode \"79110\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:street \"Georges-Köhler-Allee\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:building \"university\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:building:levels \"4\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:name \"Gebäude 51\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:98284318 osmkey:roof:levels \"1\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:roof:shape \"hipped\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 osmkey:source:outline "
                                   "\"maps4bw (LGL, www.lgl-bw.de)\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 geo:hasGeometry \"LINESTRING(7"));
  ASSERT_THAT(printedData, ::testing::HasSubstr("7)\"^^geo:wktLiteral .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "smway:98284318 geo:hasGeometry \"MULTIPOLYGON(((7"));
  ASSERT_THAT(printedData, ::testing::HasSubstr("0)))\"^^geo:wktLiteral .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:2110601105 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:2110601105 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:2110601134 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:2110601134 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:5190342871 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:5190342871 .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, tf) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.addAreaWayLinestrings = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "tf.osm");
  std::ofstream inputFile(config.input);

  std::vector<std::filesystem::path> nodes =
      getFilesWithPrefixFromPath("../../tests/e2e/tf", "n");
  std::vector<std::filesystem::path> ways =
      getFilesWithPrefixFromPath("../../tests/e2e/tf", "w");
  std::sort(nodes.begin(), nodes.end(),
            [](const auto& a, const auto& b) -> bool {
              return a.filename().string() < b.filename().string();
            });
  std::sort(ways.begin(), ways.end(), [](const auto& a, const auto& b) -> bool {
    return a.filename().string() < b.filename().string();
  });
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  for (const auto& f : nodes) {
    inputFile << std::ifstream(f).rdbuf();
  }
  for (const auto& f : ways) {
    inputFile << std::ifstream(f).rdbuf();
  }
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:45 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:1 dumped: 1 geometry: 1\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr(
                  "Skipping contains relation for nodes ... no nodes\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("decided 0 (0.000%) by DAG\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:4498466 rdf:type osm:way .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:4498466 osmkey:name \"Technische Fakultät\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:4498466 osmkey:int_name \"Faculty of Engineering\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:4498466 osmkey:operator "
                               "\"Albert-Ludwigs-Universität Freiburg\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:4498466 osmkey:wheelchair \"yes\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:4498466 geo:hasGeometry \"LINESTRING(7"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:4498466 geo:hasGeometry \"MULTIPOLYGON(((7"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}

// ____________________________________________________________________________
TEST(E2E, building51inTF) {
  // Capture std::cerr and std::cout
  std::stringstream cerrBuffer;
  std::stringstream coutBuffer;
  std::streambuf* cerrBufferOrig = std::cerr.rdbuf();
  std::streambuf* coutBufferOrig = std::cout.rdbuf();
  std::cerr.rdbuf(cerrBuffer.rdbuf());
  std::cout.rdbuf(coutBuffer.rdbuf());

  osm2rdf::config::Config config;
  config.output = "";
  config.hasGeometryAsWkt = true;
  config.outputCompress = false;
  config.addAreaWayLinestrings = true;
  config.mergeOutput = osm2rdf::util::OutputMergeMode::NONE;

  // Create empty input file
  config.input = config.getTempPath("E2E", "building51.osm");
  std::ofstream inputFile(config.input);

  std::vector<std::filesystem::path> nodes =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "n");
  const auto& tempNodes = getFilesWithPrefixFromPath("../../tests/e2e/tf", "n");
  nodes.insert(nodes.end(), tempNodes.begin(), tempNodes.end());
  std::vector<std::filesystem::path> ways =
      getFilesWithPrefixFromPath("../../tests/e2e/building_51", "w");
  const auto& tempWays = getFilesWithPrefixFromPath("../../tests/e2e/tf", "w");
  ways.insert(ways.end(), tempWays.begin(), tempWays.end());
  std::sort(nodes.begin(), nodes.end(),
            [](const auto& a, const auto& b) -> bool {
              return a.filename().string() < b.filename().string();
            });
  std::sort(ways.begin(), ways.end(), [](const auto& a, const auto& b) -> bool {
    return a.filename().string() < b.filename().string();
  });
  // Copied from ...
  inputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<osm version=\"0.6\">\n";
  for (const auto& f : nodes) {
    inputFile << std::ifstream(f).rdbuf();
  }
  for (const auto& f : ways) {
    inputFile << std::ifstream(f).rdbuf();
  }
  inputFile << "</osm>" << std::endl;

  osm2rdf::util::Output output{config, config.output};
  output.open();
  osm2rdf::ttl::Writer<osm2rdf::ttl::format::QLEVER> writer{config, &output};
  writer.writeHeader();

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  output.flush();
  output.close();

  const auto printedState = cerrBuffer.str();
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("areas seen:2 dumped: 2 geometry: 2\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("nodes seen:64 dumped: 3 geometry: 3\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("relations seen:0 dumped: 0 geometry: 0\n"));
  ASSERT_THAT(printedState,
              ::testing::HasSubstr("ways seen:2 dumped: 2 geometry: 2\n"));
  ASSERT_THAT(
      printedState,
      ::testing::HasSubstr("Contains relations for 3 nodes in 2 areas ...\n"));
  ASSERT_THAT(
      printedState,
      ::testing::HasSubstr("Contains relations for 2 ways in 2 areas ...\n"));
  const auto printedData = coutBuffer.str();
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 rdf:type osm:way .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:city \"Freiburg im Breisgau\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:housenumber \"51\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:addr:postcode \"79110\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osmkey:addr:street \"Georges-Köhler-Allee\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:98284318 osmkey:building \"university\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:building:levels \"4\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:name \"Gebäude 51\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:98284318 osmkey:roof:levels \"1\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 osmkey:roof:shape \"hipped\" .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:98284318 osmkey:source:outline "
                                   "\"maps4bw (LGL, www.lgl-bw.de)\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:98284318 geo:hasGeometry \"LINESTRING(7"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "smway:98284318 geo:hasGeometry \"MULTIPOLYGON(((7"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr("osmway:4498466 rdf:type osm:way .\n"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:4498466 osmkey:name \"Technische Fakultät\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:4498466 osmkey:int_name \"Faculty of Engineering\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:4498466 osmkey:operator "
                               "\"Albert-Ludwigs-Universität Freiburg\" .\n"));
  ASSERT_THAT(printedData, ::testing::HasSubstr(
                               "osmway:4498466 osmkey:wheelchair \"yes\" .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr("osmway:4498466 geo:hasGeometry \"LINESTRING(7"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:4498466 geo:hasGeometry \"MULTIPOLYGON(((7"));
  ASSERT_THAT(printedData,
              ::testing::HasSubstr(
                  "osmway:4498466 osm2rdf:contains_area osmway:98284318 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:4498466 osm2rdf:intersects_area osmway:98284318 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:2110601105 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:2110601105 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:2110601134 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:2110601134 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:intersects_nonarea osmnode:5190342871 .\n"));
  ASSERT_THAT(
      printedData,
      ::testing::HasSubstr(
          "osmway:98284318 osm2rdf:contains_nonarea osmnode:5190342871 .\n"));

  // Reset std::cerr and std::cout
  std::cerr.rdbuf(cerrBufferOrig);
  std::cout.rdbuf(coutBufferOrig);
  std::filesystem::remove(config.input);
}
