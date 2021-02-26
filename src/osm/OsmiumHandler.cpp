// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/OsmiumHandler.h"

#include "osm2ttl/osm/FactHandler.h"
#include "osm2ttl/osm/GeometryHandler.h"
#include "osm2ttl/osm/LocationHandler.h"
#include "osm2ttl/util/Time.h"
#include "osmium/area/assembler.hpp"
#include "osmium/area/multipolygon_manager.hpp"
#include "osmium/io/any_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/util/memory.hpp"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::OsmiumHandler<W>::OsmiumHandler(
    const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer)
    : _config(config),
      _dumpHandler(osm2ttl::osm::FactHandler<W>(config, writer)),
      _geometryHandler(osm2ttl::osm::GeometryHandler<W>(config, writer)) {}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::handle() {
  osmium::io::File input_file{_config.input};
  {
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
                << "OSM Pass 2 ... (dump)" << std::endl;
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
                << "... done reading (libosmium) and converting (libosmium -> "
                   "osm2ttl)"
                << std::endl;

      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "areas seen:" << _areasSeen << " dumped: " << _areasDumped
                << " geometry: " << _areaGeometriesHandled << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "nodes seen:" << _nodesSeen << " dumped: " << _nodesDumped
                << " geometry: " << _nodeGeometriesHandled << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "relations seen:" << _relationsSeen
                << " dumped: " << _relationsDumped
                << " geometry: " << _relationGeometriesHandled << "\n"
                << osm2ttl::util::formattedTimeSpacer
                << "ways seen:" << _waysSeen << " dumped: " << _waysDumped
                << " geometry: " << _wayGeometriesHandled << std::endl;
    }

    if (!_config.noGeometricRelations) {
      std::cerr << std::endl;
      std::cerr << osm2ttl::util::currentTimeFormatted()
                << "Calculating contains relation ..." << std::endl;
      _geometryHandler.calculateRelations();
      std::cerr << osm2ttl::util::currentTimeFormatted() << "... done"
                << std::endl;
    }

    osmium::MemoryUsage memory;
    std::cerr << osm2ttl::util::formattedTimeSpacer
              << "Memory used: " << memory.peak() << " MBytes" << std::endl;
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::area(const osmium::Area& area) {
  _areasSeen++;
  if (_config.adminRelationsOnly && area.tags()["admin_level"] == nullptr) {
    return;
  }
  const auto& a = osm2ttl::osm::Area(area);
  if (!_config.noFacts && !_config.noAreaFacts) {
    _areasDumped++;
    _dumpHandler.area(a);
  }
  if (!_config.noGeometricRelations && !_config.noAreaGeometricRelations) {
    _areaGeometriesHandled++;
    _geometryHandler.area(a);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::node(const osmium::Node& node) {
  _nodesSeen++;
  if (_config.adminRelationsOnly) {
    return;
  }
  const auto& n = osm2ttl::osm::Node(node);
  if (node.tags().empty()) {
    return;
  }
  if (!_config.noFacts && !_config.noNodeFacts) {
    _nodesDumped++;
    _dumpHandler.node(n);
  }
  if (!_config.noGeometricRelations && !_config.noNodeGeometricRelations) {
    _nodeGeometriesHandled++;
    _geometryHandler.node(n);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::relation(
    const osmium::Relation& relation) {
  _relationsSeen++;
  if (relation.tags().empty()) {
    return;
  }
  if (_config.adminRelationsOnly && relation.tags()["admin_level"] == nullptr) {
    return;
  }
  const auto& r = osm2ttl::osm::Relation(relation);
  if (!_config.noFacts && !_config.noRelationFacts) {
    _relationsDumped++;
    _dumpHandler.relation(r);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::way(const osmium::Way& way) {
  _waysSeen++;
  if (way.tags().empty()) {
    return;
  }
  if (_config.adminRelationsOnly) {
    return;
  }
  const auto& w = osm2ttl::osm::Way(way);
  if (!_config.noFacts && !_config.noWayFacts) {
    _waysDumped++;
    _dumpHandler.way(w);
  }
  if (!_config.noGeometricRelations && !_config.noWayGeometricRelations) {
    _wayGeometriesHandled++;
    _geometryHandler.way(w);
  }
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::areasSeen() const {
  return _areasSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::areasDumped() const {
  return _areasDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::areaGeometriesHandled() const {
  return _areaGeometriesHandled;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::nodesSeen() const {
  return _nodesSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::nodesDumped() const {
  return _nodesDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::nodeGeometriesHandled() const {
  return _nodeGeometriesHandled;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::relationsSeen() const {
  return _relationsSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::relationsDumped() const {
  return _relationsDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::relationGeometriesHandled() const {
  return _relationGeometriesHandled;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::waysSeen() const {
  return _waysSeen;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::waysDumped() const {
  return _waysDumped;
}

// ____________________________________________________________________________
template <typename W>
size_t osm2ttl::osm::OsmiumHandler<W>::wayGeometriesHandled() const {
  return _wayGeometriesHandled;
}

// ____________________________________________________________________________
template class osm2ttl::osm::OsmiumHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::OsmiumHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::OsmiumHandler<osm2ttl::ttl::format::QLEVER>;