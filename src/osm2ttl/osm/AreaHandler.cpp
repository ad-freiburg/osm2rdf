// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaHandler.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"

// ____________________________________________________________________________
osm2ttl::osm::AreaHandler::AreaHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer* writer) : _config(config), _writer(writer) {
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::area(const osmium::Area& area) {
  _areas.emplace(osm2ttl::osm::Area(area));
}

// ____________________________________________________________________________
void osm2ttl::osm::AreaHandler::sort() {
  for (const auto& a : _areas) {
    std::cout << "Area " << a.id() << " (" << a.objId() << ") :: " << a.bbox()
      << "\n";
    std::cout << "area " << std::setprecision(17) << a.vagueArea()
      << " -> " << std::setprecision(17) << a.area() << "\n";
    for (auto& o : a.rings()) {
      std::cout << "(OUTER" << "\n";
      for (auto& v : o.vertices) {
        std::cout << "  " << v << "\n";
      }
      for (auto& i : o.inner) {
        std::cout << "  [INNER" << "\n";
        for (auto& v : i.vertices) {
          std::cout << "    " << v << "\n";
        }
      std::cout << "  ]" << "\n";
      }
      std::cout << ")" << "\n";
      std::cout << "\n";
    }
    std::cout << "\n";
    return;
  }
}
