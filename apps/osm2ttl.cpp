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

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "osm2ttl/Version.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/config/ExitCode.h"
#include "osm2ttl/osm/OsmiumHandler.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/Ram.h"
#include "osm2ttl/util/Time.h"

template <typename T>
void run(const osm2ttl::config::Config& config) {
  // Setup
  // Input file reference
  osm2ttl::util::Output output{config, config.output};
  if (!output.open()) {
    std::cerr << "Error opening outputfile: " << config.output << std::endl;
    exit(1);
  }
  osm2ttl::ttl::Writer<T> writer{config, &output};
  writer.writeHeader();

  osm2ttl::osm::OsmiumHandler osmiumHandler{config, &writer};
  osmiumHandler.handle();

  // All work done, close output
  output.close();
}

// ____________________________________________________________________________
int main(int argc, char** argv) {
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: BEGIN"
            << std::endl;
  osm2ttl::config::Config config;
  config.fromArgs(argc, argv);
  std::cerr << config.getInfo(osm2ttl::util::formattedTimeSpacer) << std::endl;

  std::cerr << osm2ttl::util::currentTimeFormatted() << "Free ram: "
            << osm2ttl::util::ram::available() /
                   (osm2ttl::util::ram::GIGA * 1.0)
            << "G/"
            << osm2ttl::util::ram::physPages() /
                   (osm2ttl::util::ram::GIGA * 1.0)
            << "G" << std::endl;

  try {
    if (config.outputFormat == "qlever") {
      run<osm2ttl::ttl::format::QLEVER>(config);
    } else if (config.outputFormat == "nt") {
      run<osm2ttl::ttl::format::NT>(config);
    } else if (config.outputFormat == "ttl") {
      run<osm2ttl::ttl::format::TTL>(config);
    } else {
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: ERROR"
                << std::endl;
      std::cerr << "Unknown output format: " << config.outputFormat
                << std::endl;
      std::exit(osm2ttl::config::ExitCode::FAILURE);
    }
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: ERROR"
              << std::endl;
    std::cerr << e.what() << std::endl;
    std::exit(osm2ttl::config::ExitCode::EXCEPTION);
  }
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: FINISHED"
            << std::endl;
  std::exit(osm2ttl::config::ExitCode::SUCCESS);
}
