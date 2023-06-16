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

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "osm2rdf/Version.h"
#include "osm2rdf/config/Config.h"
#include "osm2rdf/config/ExitCode.h"
#include "osm2rdf/osm/OsmiumHandler.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/Ram.h"
#include "osm2rdf/util/Time.h"

// ____________________________________________________________________________
template <typename T>
void run(const osm2rdf::config::Config& config) {
  // Setup
  // Input file reference
  osm2rdf::util::Output output{config, config.output};
  if (!output.open()) {
    std::cerr << "Error opening outputfile: " << config.output << std::endl;
    exit(1);
  }
  osm2rdf::ttl::Writer<T> writer{config, &output};
  writer.writeHeader();
  writer.writeTriple(
      osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
      writer.generateIRIUnsafe(osm2rdf::ttl::constants::NAMESPACE__OSM2RDF,
                               "git-info"),
      osm2rdf::version::GIT_INFO);

  osm2rdf::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  // All work done, close output
  output.close();

  // Write final RDF statistics if requested
  if (config.writeRDFStatistics) {
    writer.writeStatisticJson(config.rdfStatisticsPath);
  }
}

// ____________________________________________________________________________
int main(int argc, char** argv) {
  std::cerr << osm2rdf::util::currentTimeFormatted()
            << "osm2rdf :: " << osm2rdf::version::GIT_INFO << " :: BEGIN"
            << std::endl;
  osm2rdf::config::Config config;
  config.fromArgs(argc, argv);
  std::cerr << config.getInfo(osm2rdf::util::formattedTimeSpacer) << std::endl;

  std::cerr << osm2rdf::util::currentTimeFormatted() << "Free ram: "
            << osm2rdf::util::ram::available() /
                   (osm2rdf::util::ram::GIGA * 1.0)
            << "G/"
            << osm2rdf::util::ram::physPages() /
                   (osm2rdf::util::ram::GIGA * 1.0)
            << "G" << std::endl;

  try {
    if (config.outputFormat == "qlever") {
      run<osm2rdf::ttl::format::QLEVER>(config);
    } else if (config.outputFormat == "nt") {
      run<osm2rdf::ttl::format::NT>(config);
    } else if (config.outputFormat == "ttl") {
      run<osm2rdf::ttl::format::TTL>(config);
    } else {
      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "osm2rdf :: " << osm2rdf::version::GIT_INFO << " :: ERROR"
                << std::endl;
      std::cerr << "Unknown output format: " << config.outputFormat
                << std::endl;
      std::exit(osm2rdf::config::ExitCode::FAILURE);
    }
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << osm2rdf::util::currentTimeFormatted()
              << "osm2rdf :: " << osm2rdf::version::GIT_INFO << " :: ERROR"
              << std::endl;
    std::cerr << e.what() << std::endl;
    std::exit(osm2rdf::config::ExitCode::EXCEPTION);
  }
  std::cerr << osm2rdf::util::currentTimeFormatted()
            << "osm2rdf :: " << osm2rdf::version::GIT_INFO << " :: FINISHED"
            << std::endl;
  std::exit(osm2rdf::config::ExitCode::SUCCESS);
}
