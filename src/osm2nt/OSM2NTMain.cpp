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
#include "osm2nt/nt/Writer.h"
#include "osm2nt/osm/DumpHandler.h"

// ____________________________________________________________________________
int main(int argc, char** argv) {
  setbuf(stdout, NULL);
  std::setlocale(LC_ALL, "en_US.UTF-8");

  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: osm2nt <pbf_input> [<outfile>]" << std::endl;
    std::cerr << VERSION_FULL << std::endl;
    std::exit(1);
  }

  std::ostream* os = &std::cout;
  std::ofstream outFile;
  if (argc == 3) {
    outFile.open(argv[2]);
    os = &outFile;
  }

  try {
    // Input file reference
    osmium::io::File input_file{argv[1], "pbf"};
    osm2nt::osm::DumpHandler dump_handler{os};

    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler>
      mp_manager{assembler_config};

    // Read relations for areas
    {
      osmium::io::Reader reader{input_file};
      osmium::ProgressBar progress{reader.file_size(), osmium::isatty(2)};
      std::cout << "Pass 1 ... (Relations for areas)" << std::endl;
      osmium::relations::read_relations(progress, input_file, mp_manager);
      std::cout << "... done" << std::endl;
    }

    std::cerr << "Memory:\n";
    osmium::relations::print_used_memory(std::cerr, mp_manager.used_memory());

    std::string cache_filename = "/tmp/osmcache";
    const int fd = ::open(cache_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC,
          0666);
    if (fd == -1) {
      std::cerr << "Can not open location cache file '" << cache_filename
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
      std::cout << "Pass 2 ... (all)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
        osmium::osm_entity_bits::object};
      osmium::apply(reader, location_handler,
        mp_manager.handler([&dump_handler](osmium::memory::Buffer&& buffer) {
          osmium::apply(buffer, dump_handler);
      }), dump_handler);
      reader.close();
      std::cout << "... done" << std::endl;
    }

    osmium::MemoryUsage memory;
    std::cout << "Memory used: " << memory.peak() << " MBytes" << std::endl;
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }
}
