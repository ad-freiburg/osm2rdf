// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "osm2ttl/Version.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/DumpHandler.h"
#include "osm2ttl/osm/GeometryHandler.h"
#include "osm2ttl/osm/LocationHandler.h"
#include "osm2ttl/osm/OsmiumHandler.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/util/Ram.h"
#include "osm2ttl/util/Time.h"
#include "osmium/area/assembler.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/util/memory.hpp"

template <typename T>
void run(osm2ttl::config::Config& config) {
  // Setup
  // Input file reference
  osmium::io::File input_file{config.input};
  osm2ttl::ttl::Writer<T> writer{config};
  if (!writer.open()) {
    std::cerr << "Error opening outputfile: " << config.output << std::endl;
    exit(1);
  }
  writer.writeHeader();

  osm2ttl::osm::OsmiumHandler osmiumHandler{config, &writer};

  {
    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{
        assembler_config};

    // read relations for areas
    {
      std::cerr << std::endl;
      osmium::io::Reader reader{input_file};
      osmium::ProgressBar progress{reader.file_size(), osmium::isatty(2)};
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "OSM Pass 1 ... (Relations for areas)" << std::endl;
      osmium::relations::read_relations(progress, input_file, mp_manager);
      std::cerr << osm2ttl::util::currentTimeFormatted() << "... done"
                << std::endl;
    }

    // store data
    {
      std::cerr << std::endl;
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "OSM Pass 2 ... (dump)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
                                               osmium::osm_entity_bits::object};
      osm2ttl::osm::LocationHandler* locationHandler =
          osm2ttl::osm::LocationHandler::create(config);
      while (true) {
        osmium::memory::Buffer buf = reader.read();
        if (!buf) {
          break;
        }
        osmium::apply(buf, *locationHandler,
                      mp_manager.handler(
                          [&osmiumHandler](osmium::memory::Buffer&& buffer) {
                            osmium::apply(buffer, osmiumHandler);
                          }),
                      osmiumHandler);
      }
      reader.close();
      delete locationHandler;
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "... done reading (libosmium) and converting (libosmium -> "
                   "osm2ttl)"
                << std::endl;
    }

    {
      std::cerr << std::endl;
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "Calculating contains relation ..." << std::endl;
      osmiumHandler.calculateRelations();
      std::cerr << osm2ttl::util::currentTimeFormatted() << "... done"
                << std::endl;
    }
  }

  // All work done, close output
  writer.close();

  osmium::MemoryUsage memory;
  std::cerr << osm2ttl::util::formattedTimeSpacer
            << "Memory used: " << memory.peak() << " MBytes" << std::endl;
}

// ____________________________________________________________________________
int main(int argc, char** argv) {
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: BEGIN"
            << std::endl;
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
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
      std::exit(1);
    }
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: ERROR"
              << std::endl;
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << "osm2ttl :: " << osm2ttl::version::GIT_INFO << " :: FINISHED"
            << std::endl;
  std::exit(0);
}
