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

#include "osm2ttl/_config.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/ttl/Writer.h"
#include "osm2ttl/osm/AreaHandler.h"
#include "osm2ttl/osm/DumpHandler.h"

// ____________________________________________________________________________
int main(int argc, char** argv) {
  osm2ttl::config::Config config;
  config.fromArgs(argc, argv);

  try {
    // Setup
    // Input file reference
    osmium::io::File input_file{config.input};
    osm2ttl::ttl::Writer writer{config};
    if (!writer.open()) {
      std::cerr << "Error opening outputfile: " << config.output << std::endl;
      exit(1);
    }
    writer.writeHeader();
    osm2ttl::osm::DumpHandler dump_handler{&writer};

    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler>
      mp_manager{assembler_config};

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
    osm2ttl::osm::AreaHandler area_handler{config, &writer, &location_handler};

    location_handler.ignore_errors();

    // Do the work
    // First pass: read relations for areas
    {
      osmium::io::Reader reader{input_file};
      osmium::ProgressBar progress{reader.file_size(), osmium::isatty(2)};
      std::cerr << "Pass 1 ... (Relations for areas)" << std::endl;
      osmium::relations::read_relations(progress, input_file, mp_manager);
      std::cerr << "... done" << std::endl;
    }

    std::cerr << "Memory:\n";
    osmium::relations::print_used_memory(std::cerr, mp_manager.used_memory());

    // Second pass: handle all data
    {
      std::cerr << "Pass 2 ... (all)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
        osmium::osm_entity_bits::object};
      osmium::apply(reader, location_handler,
        mp_manager.handler([&dump_handler, &area_handler](
            osmium::memory::Buffer&& buffer) {
          osmium::apply(buffer, dump_handler);
          osmium::apply(buffer, area_handler);
      }), dump_handler);
      reader.close();
      std::cerr << "... done" << std::endl;
    }

    std::cerr << "Memory:\n";
    osmium::relations::print_used_memory(std::cerr, mp_manager.used_memory());

    osmium::MemoryUsage memory;
    std::cerr << "Memory used: " << memory.peak() << " MBytes" << std::endl;

    std::cerr << "Location index:  ~"
      << (index.used_memory() / (1024 * 1024)) << " MBytes"
      << std::endl;

    // All data stored and partialy handled, calculate additional data
    // area_handler.calculate();

    // All work done, close output
    writer.close();
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }
}
