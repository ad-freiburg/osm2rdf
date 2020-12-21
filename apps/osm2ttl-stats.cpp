// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include <cstdlib>
#include <iostream>
#include <string>

#include "osm2ttl/Version.h"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/LocationHandler.h"
#include "osm2ttl/util/Time.h"
#include "osmium/area/assembler.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/util/memory.hpp"

class OsmiumIdHandler : public osmium::handler::Handler {
 public:
  explicit OsmiumIdHandler(const osm2ttl::config::Config& config)
      : _config(config){};

  void handle() {
    osmium::io::File input_file{_config.input};
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
                << "OSM Pass 2 ... (id lookup)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
                                               osmium::osm_entity_bits::object};
      osm2ttl::osm::LocationHandler* locationHandler =
          osm2ttl::osm::LocationHandler::create(_config);
      while (true) {
        osmium::memory::Buffer buf = reader.read();
        if (!buf) {
          break;
        }
        osmium::apply(buf, *locationHandler,
                      mp_manager.handler([&](osmium::memory::Buffer&& buffer) {
                        osmium::apply(buffer, *this);
                      }),
                      *this);
      }
      reader.close();
      delete locationHandler;
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "... done reading (libosmium)" << std::endl;

      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "max area id:     " << _maxAreaId << " "
                << idInfo(_maxAreaId) << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "max node id:     " << _maxNodeId << " "
                << idInfo(_maxNodeId) << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "max relation id: " << _maxRelationId << " "
                << idInfo(_maxRelationId) << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "max way id:      " << _maxWayId << " " << idInfo(_maxWayId)
                << std::endl;

      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "num areas:     " << _countAreas << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "num nodes:     " << _countNodes << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "num relations: " << _countRelations << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "num ways:      " << _countWays << std::endl;

      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "num required nodes:     " << _requiredNodes.size() << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "num required relations: " << _requiredRelations.size()
                << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "num required ways:      " << _requiredWays.size()
                << std::endl;
    }
  }

  void area(const osmium::Area& area) {
    _countAreas++;
    _maxAreaId = std::max(_maxAreaId, area.positive_id());
    for (const auto& outer : area.outer_rings()) {
      for (const auto& node_ref : outer) {
        _requiredNodes.insert(node_ref.positive_ref());
      }
      for (const auto& inner : area.inner_rings(outer)) {
        for (const auto& node_ref : inner) {
          _requiredNodes.insert(node_ref.positive_ref());
        }
      }
    }
  }

  void node(const osmium::Node& node) {
    _countNodes++;
    _maxNodeId = std::max(_maxNodeId, node.positive_id());
  }

  void relation(const osmium::Relation& relation) {
    _countRelations++;
    _maxRelationId = std::max(_maxRelationId, relation.positive_id());
    for (const auto& member : relation.members()) {
      switch (member.type()) {
        case osmium::item_type::node:
          _requiredNodes.insert(member.positive_ref());
          break;
        case osmium::item_type::relation:
          _requiredRelations.insert(member.positive_ref());
          break;
        case osmium::item_type::way:
          _requiredWays.insert(member.positive_ref());
          break;
        default:
          break;
      }
    }
  }

  void way(const osmium::Way& way) {
    _countWays++;
    _maxWayId = std::max(_maxWayId, way.positive_id());
    for (const auto& node_ref : way.nodes()) {
      _requiredNodes.insert(node_ref.positive_ref());
    }
  }

  static std::string idInfo(const uint64_t max) {
    std::string s{"<= "};
    if (max <= std::numeric_limits<uint8_t>::max()) {
      s += " uint8_t";
      return s;
    }
    if (max <= std::numeric_limits<uint16_t>::max()) {
      s += "uint16_t";
      return s;
    }
    if (max <= std::numeric_limits<uint32_t>::max()) {
      s += "uint32_t";
      return s;
    }
    if (max <= std::numeric_limits<uint64_t>::max()) {
      s += "uint64_t";
      return s;
    }
    s += "unknown!";
    return s;
  }

 protected:
  osm2ttl::config::Config _config;
  uint64_t _countAreas = 0;
  uint64_t _maxAreaId = 0;
  uint64_t _countNodes = 0;
  uint64_t _maxNodeId = 0;
  uint64_t _countRelations = 0;
  uint64_t _maxRelationId = 0;
  uint64_t _countWays = 0;
  uint64_t _maxWayId = 0;

  std::unordered_set<uint64_t> _requiredNodes;
  std::unordered_set<uint64_t> _requiredRelations;
  std::unordered_set<uint64_t> _requiredWays;
};

// ____________________________________________________________________________
int main(int argc, char** argv) {
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << "osm2ttl-stats :: " << osm2ttl::version::GIT_INFO << " :: BEGIN"
            << std::endl;
  osm2ttl::config::Config& config = osm2ttl::config::Config::getInstance();
  config.fromArgs(argc, argv);
  std::cerr << config.getInfo(osm2ttl::util::formattedTimeSpacer) << std::endl;

  try {
    OsmiumIdHandler osmiumIdHandler{config};
    osmiumIdHandler.handle();
  } catch (const std::exception& e) {
    // All exceptions used by the Osmium library derive from std::exception.
    std::cerr << osm2ttl::util::currentTimeFormatted()
              << "osm2ttl-stats :: " << osm2ttl::version::GIT_INFO
              << " :: ERROR" << std::endl;
    std::cerr << e.what() << std::endl;
    std::exit(1);
  }
  std::cerr << osm2ttl::util::currentTimeFormatted()
            << "osm2ttl-stats :: " << osm2ttl::version::GIT_INFO
            << " :: FINISHED" << std::endl;
  std::exit(0);
}