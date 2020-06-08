// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <string>

#include "osmium/area/assembler.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/handler/node_locations_for_ways.hpp"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/util/memory.hpp"

#include "osm2nt/_config.h"
#include "osm2nt/config/Config.h"
#include "osm2nt/config/ConfigParser.h"
#include "osm2nt/nt/Writer.h"
#include "osm2nt/osm/DumpHandler.h"

// ____________________________________________________________________________
int main(int argc, char** argv) {
  osm2nt::config::Config config;
  osm2nt::config::ConfigParser::parse(&config, argc, argv);

  try {
    // Input file reference
    osmium::io::File input_file{config.input};
    osm2nt::osm::DumpHandler dump_handler{config};

    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler>
      mp_manager{assembler_config};

    // Read relations for areas
    {
      osmium::io::Reader reader{input_file};
      osmium::ProgressBar progress{reader.file_size(), osmium::isatty(2)};
      std::cerr << "Pass 1 ... (Relations for areas)" << std::endl;
      osmium::relations::read_relations(progress, input_file, mp_manager);
      std::cerr << "... done" << std::endl;
    }

    std::cerr << "Memory:\n";
    osmium::relations::print_used_memory(std::cerr, mp_manager.used_memory());

    const int fd = ::open(config.cache.c_str(), O_RDWR | O_CREAT | O_TRUNC,
          0666);
    if (fd == -1) {
      std::cerr << "Can not open location cache file '" << config.cache
        << "': " << std::strerror(errno) << "\n";
      std::exit(1);
    }
    osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location> index{fd};
    osmium::handler::NodeLocationsForWays<
      osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>
      location_handler{index};

    location_handler.ignore_errors();

    {
      std::cerr << "Pass 2 ... (all)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
        osmium::osm_entity_bits::object};
      osmium::apply(reader, location_handler,
        mp_manager.handler([&dump_handler](osmium::memory::Buffer&& buffer) {
          osmium::apply(buffer, dump_handler);
      }), dump_handler);
      reader.close();
      std::cerr << "... done" << std::endl;
    }

    std::cerr << "Memory:\n";
    osmium::relations::print_used_memory(std::cerr, mp_manager.used_memory());

    osmium::MemoryUsage memory;
    std::cerr << "Memory used: " << memory.peak() << " MBytes" << std::endl;
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }
}
