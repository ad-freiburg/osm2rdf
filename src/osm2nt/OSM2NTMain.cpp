// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <clocale>
#include <cstdlib>
#include <iostream>

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/io/pbf_input.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/util/memory.hpp>

class WikiHandler : public osmium::handler::Handler {
 public:
  void osm_object(const osmium::OSMObject& osm_object) {
    const osmium::TagList& tags = osm_object.tags();
    const char* wikidata = tags["wikidata"];
    const char* wikipedia = tags["wikipedia"];
    const char* name = tags["name"];

    /*if (name) {
      std::ostringstream result;
      result << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
      std::string s_name = name;
      std::copy(s_name.begin(), s_name.end(), std::ostream_iterator<unsigned int>(result, " "));
      std::cout << "     name: " << result.str() << std::endl;
    }
    if (wikidata) {
      std::ostringstream result;
      result << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
      std::string s_wikidata = wikidata;
      std::copy(s_wikidata.begin(), s_wikidata.end(), std::ostream_iterator<unsigned int>(result, " "));
      std::cout << " wikidata: " << result.str() << std::endl;
    }
    if (wikipedia) {
      std::ostringstream result;
      result << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
      std::string s_wikipedia = wikipedia;
      std::copy(s_wikipedia.begin(), s_wikipedia.end(), std::ostream_iterator<unsigned int>(result, " "));
      std::cout << "wikipedia: " << result.str() << std::endl;
    }*/

    if (name || wikidata || wikipedia) {
      if (name) {
        std::cout << name;
      } else {
        std::cout << "Unknown name";
      }
      if (wikidata) {
        std::cout << " " << wikidata;
      }
      if (wikipedia) {
        std::cout << " " << wikipedia;
      }
      std::cout << std::endl;
    }
  }
};

// ____________________________________________________________________________
int main(int argc, char** argv) {
  setbuf(stdout, NULL);
  std::setlocale(LC_ALL, "en_US.UTF-8");

  if (argc != 2) {
    std::cerr << "Usage: osm2nt <pbf_input>" << std::endl;
    std::exit(1);
  }

  try {
    // Input file reference
    osmium::io::File input_file{argv[1], "pbf"};
    WikiHandler wiki_handler;

    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{assembler_config};

    // Read relations for areas
    {
      osmium::io::Reader reader{input_file};
      reader.close();
      std::cout << "Pass 1 ... (Relations for areas)" << std::endl;
      osmium::relations::read_relations(input_file, mp_manager);
      std::cout << "... done" << std::endl;
    }

    {
      std::cout << "Pass 2 ... (Ways)" << std::endl;
      osmium::io::Reader reader{input_file, osmium::osm_entity_bits::way};
      //osmium::apply(reader, wiki_handler);
      reader.close();
      std::cout << "... done" << std::endl;
    }

    {
      std::cout << "Pass 3 ... (Nodes)" << std::endl;
      osmium::io::Reader reader{input_file, osmium::osm_entity_bits::node};
      //osmium::apply(reader, wiki_handler);
      reader.close();
      std::cout << "... done" << std::endl;
    }

    {
      std::cout << "Pass 4 ... (all)" << std::endl;
      osmium::io::Reader reader{input_file, osmium::osm_entity_bits::object};
      osmium::apply(reader, wiki_handler);
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
