// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaHandler.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>

#include "osmium/osm/area.hpp"
#include "osmium/index/map/sparse_file_array.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"

// ____________________________________________________________________________
osm2ttl::osm::AreaHandler::AreaHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer,
  osmium::handler::NodeLocationsForWays<
    osmium::index::map::SparseFileArray<
      osmium::unsigned_object_id_type, osmium::Location>>* nodeLocations) :
  _config(config), _writer(writer), _nodeLocations(nodeLocations) {
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::area(const osmium::Area& area) {
  _areas.emplace(_areaCount,
                 osm2ttl::osm::Area(_areaCount, area, _nodeLocations));
  _areaCount++;
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::calculate() {
  for (auto& pair : _areas) {
    osm2ttl::osm::Area& a = pair.second;
    std::cout << "Area " << a.id() << " (" << a.objId() << ") :: " << a.bbox()
      << "\n";
    std::cout << "area " << std::setprecision(17) << a.vagueArea()
      << " -> " << std::setprecision(17) << a.area() << "\n";
    /**
    for (auto& o : a.rings()) {
      std::cout << "(OUTER" << "\n";
      for (auto& n : o.nodes) {
        std::cout << "  " << _nodeLocations->get_node_location(n) << "\n";
      }
      for (auto& i : o.inner) {
        std::cout << "  [INNER" << "\n";
        for (auto& n : i.nodes) {
          std::cout << "    " << _nodeLocations->get_node_location(n) << "\n";
        }
      std::cout << "  ]" << "\n";
      }
      std::cout << ")" << "\n";
      std::cout << "\n";
    }
    */
    std::cout << "\n";
  }
}
