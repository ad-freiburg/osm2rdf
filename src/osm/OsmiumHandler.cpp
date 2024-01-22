// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>.

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#include "osm2rdf/osm/OsmiumHandler.h"

#include "boost/version.hpp"
#include "osm2rdf/osm/CountHandler.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/GeometryHandler.h"
#include "osm2rdf/osm/LocationHandler.h"
#include "osm2rdf/osm/RelationHandler.h"
#include "osm2rdf/util/ProgressBar.h"
#include "osm2rdf/util/Time.h"
#include "osmium/area/assembler.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/util/memory.hpp"

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::OsmiumHandler<W>::OsmiumHandler(
    const osm2rdf::config::Config& config, osm2rdf::ttl::Writer<W>* writer)
    : _config(config),
      _factHandler(osm2rdf::osm::FactHandler<W>(config, writer)),
      _geometryHandler(osm2rdf::osm::GeometryHandler<W>(config, writer)),
      _relationHandler(osm2rdf::osm::RelationHandler(config)) {}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::handle() {
  osmium::io::File input_file{_config.input};
  {
    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{
        assembler_config};
    osm2rdf::osm::CountHandler countHandler;

    // read relations for areas
    {
      std::cerr << std::endl;
      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "OSM Pass 1 ... (Count objects, Relations for areas"
#if BOOST_VERSION >= 107800
                << ", Relation members"
#endif  // BOOST_VERSION >= 107800
                << ")" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
                                               osmium::osm_entity_bits::object};
      {
        while (auto buf = reader.read()) {
          osmium::apply(buf, mp_manager,
#if BOOST_VERSION >= 107800
                        _relationHandler,
#endif  // BOOST_VERSION >= 107800
                        countHandler);
        }
      }
      reader.close();
      mp_manager.prepare_for_lookup();
#if BOOST_VERSION >= 107800
      _relationHandler.prepare_for_lookup();
#endif  // BOOST_VERSION >= 107800
      std::cerr << osm2rdf::util::currentTimeFormatted() << "... done"
                << std::endl;
    }

    // store data
    {
      std::cerr << std::endl;
      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "OSM Pass 2 ... (dump)" << std::endl;
      osmium::io::Reader reader{input_file, osmium::osm_entity_bits::object};
      osm2rdf::osm::LocationHandler* locationHandler =
          osm2rdf::osm::LocationHandler::create(_config);
      _relationHandler.setLocationHandler(locationHandler);

      size_t numTasks = 0;
      if (!_config.noFacts && !_config.noNodeFacts) {
        numTasks += countHandler.numNodes();
      }
      if (!_config.noGeometricRelations && !_config.noNodeGeometricRelations) {
        numTasks += countHandler.numNodes();
      }
      if (!_config.noFacts && !_config.noRelationFacts) {
        numTasks += countHandler.numRelations();
      }
      if (!_config.noGeometricRelations &&
          !_config.noRelationGeometricRelations) {
        numTasks += countHandler.numRelations();
      }
      if (!_config.noFacts && !_config.noWayFacts) {
        numTasks += countHandler.numWays();
      }
      if (!_config.noGeometricRelations && !_config.noWayGeometricRelations) {
        numTasks += countHandler.numWays();
      }

      _progressBar = osm2rdf::util::ProgressBar{numTasks, true};
      _progressBar.update(_numTasksDone);

#pragma omp parallel
      {
#pragma omp single
        {
          while (auto buf = reader.read()) {
            osmium::apply(
                buf, *locationHandler,
#if BOOST_VERSION >= 107800
                _relationHandler,
#endif  // BOOST_VERSION >= 107800
                mp_manager.handler([&](osmium::memory::Buffer&& buffer) {
                  osmium::apply(buffer, *this);
                }),
                *this);
          }
        }
      }
      reader.close();
      delete locationHandler;
      _progressBar.done();

      std::cerr << osm2rdf::util::currentTimeFormatted() << "... done"
                << std::endl;

      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "areas seen:" << _areasSeen << " dumped: " << _areasDumped
                << " geometry: " << _areaGeometriesHandled << "\n"
                << osm2rdf::util::formattedTimeSpacer
                << "nodes seen:" << _nodesSeen << " dumped: " << _nodesDumped
                << " geometry: " << _nodeGeometriesHandled << "\n"
                << osm2rdf::util::formattedTimeSpacer
                << "relations seen:" << _relationsSeen
                << " dumped: " << _relationsDumped
                << " geometry: " << _relationGeometriesHandled << "\n"
                << osm2rdf::util::formattedTimeSpacer
                << "ways seen:" << _waysSeen << " dumped: " << _waysDumped
                << " geometry: " << _wayGeometriesHandled << std::endl;
    }

    if (!_config.noGeometricRelations) {
      std::cerr << std::endl;
      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "Calculating contains relation ..." << std::endl;
      _geometryHandler.calculateRelations();
      std::cerr << osm2rdf::util::currentTimeFormatted() << "... done"
                << std::endl;
    }

    osmium::MemoryUsage memory;
    std::cerr << osm2rdf::util::formattedTimeSpacer
              << "Memory used: " << memory.peak() << " MBytes" << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::area(const osmium::Area& area) {
  _areasSeen++;
  if (_config.adminRelationsOnly && area.tags()["admin_level"] == nullptr) {
    return;
  }
  try {
    auto osmArea = osm2rdf::osm::Area(area);
#pragma omp task
    {
      osmArea.finalize();
      if (!_config.noFacts && !_config.noAreaFacts) {
        _areasDumped++;
#pragma omp task
        { _factHandler.area(osmArea); };
      }
      if (!_config.noGeometricRelations && !_config.noAreaGeometricRelations) {
        _areaGeometriesHandled++;
#pragma omp task
        { _geometryHandler.area(osmArea); };
      }
    }
  } catch (const osmium::invalid_location& e) {
    return;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::node(const osmium::Node& node) {
  _nodesSeen++;
  if (_config.adminRelationsOnly) {
    return;
  }
  try {
    const auto& osmNode = osm2rdf::osm::Node(node);
    if (node.tags().empty()) {
      return;
    }
    if (!_config.noFacts && !_config.noNodeFacts) {
      _nodesDumped++;
#pragma omp task
      {
        _factHandler.node(osmNode);
#pragma omp critical(progress)
        _progressBar.update(_numTasksDone++);
      };
    }
    if (!_config.noGeometricRelations && !_config.noNodeGeometricRelations) {
      _nodeGeometriesHandled++;
#pragma omp task
      {
        _geometryHandler.node(osmNode);
#pragma omp critical(progress)
        _progressBar.update(_numTasksDone++);
      };
    }
  } catch (const osmium::invalid_location& e) {
    return;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::relation(
    const osmium::Relation& relation) {
  _relationsSeen++;
  if (relation.tags().empty()) {
    return;
  }
  if (_config.adminRelationsOnly && relation.tags()["admin_level"] == nullptr) {
    return;
  }
  try {
    auto osmRelation = osm2rdf::osm::Relation(relation);
#if BOOST_VERSION >= 107800
    // only task this away if we actually build the relation geometries,
    // otherwise this just adds multithreading overhead for nothing
#pragma omp task
    {
      if (_relationHandler.hasLocationHandler()) {
        osmRelation.buildGeometry(_relationHandler);
      }
#endif  // BOOST_VERSION >= 107800
      if (!_config.noFacts && !_config.noRelationFacts) {
        _relationsDumped++;
#pragma omp task
        {
          _factHandler.relation(osmRelation);
#pragma omp critical(progress)
          _progressBar.update(_numTasksDone++);
        };
      }

      if (!_config.noGeometricRelations &&
          !_config.noRelationGeometricRelations) {
#pragma omp task
        {
          _geometryHandler.relation(osmRelation);
#pragma omp critical(progress)
          _progressBar.update(_numTasksDone++);
        };
      }
#if BOOST_VERSION >= 107800
    }
#endif
  } catch (const osmium::invalid_location& e) {
    return;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::way(const osmium::Way& way) {
  _waysSeen++;
  if (way.tags().empty()) {
    return;
  }
  if (_config.adminRelationsOnly) {
    return;
  }
  try {
    const auto& osmWay = osm2rdf::osm::Way(way);

    if (!_config.noFacts && !_config.noWayFacts) {
      _waysDumped++;
#pragma omp task
      {
        _factHandler.way(osmWay);
#pragma omp critical(progress)
        _progressBar.update(_numTasksDone++);
      };
    }
    if (!_config.noGeometricRelations && !_config.noWayGeometricRelations) {
      _wayGeometriesHandled++;
#pragma omp task
      {
        _geometryHandler.way(osmWay);
#pragma omp critical(progress)
        _progressBar.update(_numTasksDone++);
      };
    }
  } catch (const osmium::invalid_location& e) {
    return;
  }
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::areasSeen() const {
  return _areasSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::areasDumped() const {
  return _areasDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::areaGeometriesHandled() const {
  return _areaGeometriesHandled;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::nodesSeen() const {
  return _nodesSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::nodesDumped() const {
  return _nodesDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::nodeGeometriesHandled() const {
  return _nodeGeometriesHandled;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::relationsSeen() const {
  return _relationsSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::relationsDumped() const {
  return _relationsDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::relationGeometriesHandled() const {
  return _relationGeometriesHandled;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::waysSeen() const {
  return _waysSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::waysDumped() const {
  return _waysDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2rdf::osm::OsmiumHandler<W>::wayGeometriesHandled() const {
  return _wayGeometriesHandled;
}

// ____________________________________________________________________________
template class osm2rdf::osm::OsmiumHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::OsmiumHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::OsmiumHandler<osm2rdf::ttl::format::QLEVER>;
