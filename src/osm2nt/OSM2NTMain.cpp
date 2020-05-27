// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/util/memory.hpp>

#include <clocale>
#include <cstdlib>
#include <iostream>

#include "osm2nt/_config.h"
#include "osm2nt/nt/Writer.h"

class DumpHandler : public osmium::handler::Handler {
 public:
  explicit DumpHandler(std::ostream* os) {
    w = new osm2nt::nt::Writer{os};
  }
  ~DumpHandler() {
    delete w;
  }
  void relation(const osmium::Relation& relation) const {
    w->writeOsmRelation(relation);
  }
  void node(const osmium::Node& node) const {
    w->writeOsmNode(node);
  }
  void way(const osmium::Way& way) const {
    w->writeOsmWay(way);
  }
  osm2nt::nt::Writer* w;
};

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
  if (argc == 3) {
    std::ofstream outFile{argv[2]};
    os = &outFile;
  }

  try {
    // Input file reference
    osmium::io::File input_file{argv[1], "pbf"};
    DumpHandler dump_handler{os};

    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler>
      mp_manager{assembler_config};

    // Read relations for areas
    {
      osmium::io::Reader reader{input_file};
      reader.close();
      std::cout << "Pass 1 ... (Relations for areas)" << std::endl;
      osmium::relations::read_relations(input_file, mp_manager);
      std::cout << "... done" << std::endl;
    }

    {
      std::cout << "Pass 2 ... (all)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
        osmium::osm_entity_bits::object};
      osmium::apply(reader, dump_handler);
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
