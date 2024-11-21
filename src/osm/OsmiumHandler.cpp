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

#include "osm2rdf/osm/CountHandler.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/GeometryHandler.h"
#include "osm2rdf/osm/LocationHandler.h"
#include "osm2rdf/osm/OsmiumHandler.h"
#include "osm2rdf/osm/RelationHandler.h"
#include "osm2rdf/util/ProgressBar.h"
#include "osm2rdf/util/Time.h"
#include "osmium/area/assembler.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"

#if defined(_OPENMP)
#include "omp.h"
#endif

// ____________________________________________________________________________
template <typename W>
osm2rdf::osm::OsmiumHandler<W>::OsmiumHandler(
    const osm2rdf::config::Config& config,
    osm2rdf::osm::FactHandler<W>* factHandler,
    osm2rdf::osm::GeometryHandler<W>* geomHandler)
    : _config(config),
      _factHandler(factHandler),
      _geometryHandler(geomHandler),
      _relationHandler(osm2rdf::osm::RelationHandler(config)) {}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::handle() {
  {
    osmium::io::File input_file{_config.input};

    // Do not create empty areas
    osmium::area::Assembler::config_type assembler_config;
    assembler_config.create_empty_areas = false;
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{
        assembler_config};
    osm2rdf::osm::CountHandler countHandler(_config);

    // read relations for areas
    {
      std::cerr << std::endl;
      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "OSM Pass 1 ... (Count objects, Relations for areas"
                << ", Relation members)" << std::endl;
      osmium::io::ReaderWithProgressBar reader{true, input_file,
                                               osmium::osm_entity_bits::object};
      {
        while (auto buf = reader.read()) {
          osmium::apply(buf, mp_manager, _relationHandler, countHandler);
        }
      }
      reader.close();
      mp_manager.prepare_for_lookup();
      _relationHandler.prepare_for_lookup();
      std::cerr << osm2rdf::util::currentTimeFormatted() << "... done"
                << std::endl;
    }

    // store data
    {
      std::cerr << std::endl;
      std::cerr << osm2rdf::util::currentTimeFormatted()
                << "OSM Pass 2 ... (dump)" << std::endl;
      osmium::thread::Pool pool(std::max(_config.numThreads - 2, 1),
                                osmium::thread::Pool::default_queue_size);

#if defined(_OPENMP)
      omp_set_num_threads(_config.numThreads);
#endif

      osmium::io::Reader reader{input_file, osmium::osm_entity_bits::object,
                                pool};
      osm2rdf::osm::LocationHandler* locationHandler =
          osm2rdf::osm::LocationHandler::create(
              _config, countHandler.minNodeId(), countHandler.maxNodeId());
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
                buf, *locationHandler, _relationHandler,
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
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::area(const osmium::Area& area) {
  _areasSeen++;

  if (!_config.addUntaggedAreas && area.tags().empty()) {
    return;
  }

  try {
    auto osmArea = osm2rdf::osm::Area(area);
#pragma omp task
    {
      osmArea.finalize();
      if (!_config.noFacts && !_config.noAreaFacts) {
        _areasDumped++;
        _factHandler->area(osmArea);
      }
      if (!_config.noGeometricRelations && !_config.noAreaGeometricRelations) {
        _areaGeometriesHandled++;
        _geometryHandler->area(osmArea);
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

  if (!_config.addUntaggedNodes && node.tags().empty()) {
    return;
  }

  try {
    const auto& osmNode = osm2rdf::osm::Node(node);
#pragma omp task
    {
      if (!_config.noFacts && !_config.noNodeFacts) {
        _factHandler->node(osmNode);
#pragma omp critical(progress)
        {
          _nodesDumped++;
          _progressBar.update(_numTasksDone++);
        }
      }
      if (!_config.noGeometricRelations && !_config.noNodeGeometricRelations) {
        _geometryHandler->node(osmNode);
#pragma omp critical(progress)
        {
          _nodeGeometriesHandled++;
          _progressBar.update(_numTasksDone++);
        }
      }
    };
  } catch (const osmium::invalid_location& e) {
    if (!_config.noFacts && !_config.noNodeFacts) {
      _progressBar.update(_numTasksDone++);
    }
    if (!_config.noGeometricRelations && !_config.noNodeGeometricRelations) {
      _progressBar.update(_numTasksDone++);
    }
    return;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::relation(
    const osmium::Relation& relation) {
  _relationsSeen++;

  if (!_config.addUntaggedRelations && relation.tags().empty()) {
    return;
  }

  try {
    // only task this away if we actually build the relation geometries,
    // otherwise this just adds multithreading overhead for nothing
    auto osmRelation = osm2rdf::osm::Relation(relation);
#pragma omp task
    {
      if (!osmRelation.isArea() && _relationHandler.hasLocationHandler()) {
        osmRelation.buildGeometry(_relationHandler);
      }

      if (!_config.noFacts && !_config.noRelationFacts) {
        _factHandler->relation(osmRelation);
#pragma omp critical(progress)
        {
          _relationsDumped++;
          _progressBar.update(_numTasksDone++);
        }
      }

      if (!_config.noGeometricRelations &&
          !_config.noRelationGeometricRelations) {
        _geometryHandler->relation(osmRelation);
#pragma omp critical(progress)
        _progressBar.update(_numTasksDone++);
      }
    }
  } catch (const osmium::invalid_location& e) {
    if (!_config.noFacts && !_config.noRelationFacts) {
      _progressBar.update(_numTasksDone++);
    }

    if (!_config.noGeometricRelations &&
        !_config.noRelationGeometricRelations) {
      _progressBar.update(_numTasksDone++);
    }
    return;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2rdf::osm::OsmiumHandler<W>::way(const osmium::Way& way) {
  _waysSeen++;

  if (!_config.addUntaggedWays && way.tags().empty()) {
    return;
  }

  try {
    auto osmWay = osm2rdf::osm::Way(way);

#pragma omp task
    {
      if (!_config.noFacts && !_config.noWayFacts) {
        if (!osmWay.isArea()) {  // avoid double calculation of OBB and hull
          osmWay.finalize();
        }
        _factHandler->way(osmWay);
#pragma omp critical(progress)
        {
          _waysDumped++;
          _progressBar.update(_numTasksDone++);
        }
      }

      if (!_config.noGeometricRelations && !_config.noWayGeometricRelations) {
        _geometryHandler->way(osmWay);
#pragma omp critical(progress)
        {
          _wayGeometriesHandled++;
          _progressBar.update(_numTasksDone++);
        }
      }
    }
  } catch (const osmium::invalid_location& e) {
    if (!_config.noFacts && !_config.noWayFacts) {
      _progressBar.update(_numTasksDone++);
    }
    if (!_config.noGeometricRelations && !_config.noWayGeometricRelations) {
      _progressBar.update(_numTasksDone++);
    }
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
