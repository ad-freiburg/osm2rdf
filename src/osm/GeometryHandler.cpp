// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "boost/geometry.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/thread.hpp"
#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Constants.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/util/progress_bar.hpp"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::GeometryHandler<W>::GeometryHandler(
    const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer)
    : _config(config), _writer(writer) {}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::area(const osmium::Area& area) {
  if (_config.noAreaDump) {
    return;
  }
  osm2ttl::osm::Area a = osm2ttl::osm::Area(area);
  if (!a.hasName()) {
    return;
  }
  if (!a.fromWay()) {
    _spatialStorage.emplace_back(a.envelope(),
                                 std::make_pair(a.objId(), a.geom()));
  }
  if (a.tagAdministrationLevel() > 0) {
    _containingAreas.push_back(a);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::node(const osmium::Node& node) {
  if (_config.noNodeDump) {
    return;
  }
  osm2ttl::osm::Node n = osm2ttl::osm::Node(node);
  if (n.tags().count("name") < 1) {
    return;
  }
  _spatialStorage.emplace_back(n.envelope(), std::make_pair(n.id(), n.geom()));
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::way(const osmium::Way& way) {
  if (_config.noWayDump) {
    return;
  }
  osm2ttl::osm::Way w = osm2ttl::osm::Way(way);
  if (w.tags().count("name") < 1 || w.tags().count("building") < 1) {
    return;
  }
  _spatialStorage.emplace_back(w.envelope(), std::make_pair(w.id(), w.geom()));
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::prepareLookup() {
  {
    std::cerr << " Packing combined tree with " << _spatialStorage.size()
              << " entries ... " << std::flush;
    _spatialIndex =
        SpatialIndex(_spatialStorage.begin(), _spatialStorage.end());
    _spatialStorage.clear();
    std::cerr << "done" << std::endl;
    std::cerr << " Sort " << _containingAreas.size() << " containing areas ... "
              << std::flush;
    std::sort(_containingAreas.rbegin(), _containingAreas.rend());
    std::cerr << "done" << std::endl;
  }
  _sorted = true;
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::GeometryHandler<W>::lookup() {
  if (!_sorted) {
    throw std::runtime_error("GeometryHandler was not prepared for lookup!");
  }
  std::cerr << " Contains relations for " << _containingAreas.size()
            << " areas and their members ..." << std::endl;
  osmium::ProgressBar progressBar{_containingAreas.size(), true};
  size_t entryCount = 0;
  progressBar.update(entryCount);

#pragma omp parallel shared(                                            \
    entryCount, progressBar, osm2ttl::ttl::constants::IRI_OGC_CONTAINS, \
    osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,                       \
    osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,                   \
    osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
  {
#pragma omp single
    {
      for (auto& area : _containingAreas) {
        if (area.tagAdministrationLevel() > 0) {
          std::string s = _writer->generateIRI(
              area.fromWay() ? osm2ttl::ttl::constants::NAMESPACE__OSM_WAY
                             : osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
              area.objId());
          for (auto it = _spatialIndex.qbegin(
                   boost::geometry::index::covered_by(area.envelope()));
               it != _spatialIndex.qend(); it++) {
            auto entry = it->second;
#pragma omp task firstprivate(area, entry, s)                \
    shared(osm2ttl::ttl::constants::IRI_OGC_CONTAINS,        \
           osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,     \
           osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION, \
           osm2ttl::ttl::constants::NAMESPACE__OSM_WAY) default(none)
            {
              auto& entryId = entry.first;
              auto& geometry = entry.second;
              switch (geometry.index()) {
                // Handle node
                case 0:
                  if (boost::geometry::covered_by(std::get<0>(geometry),
                                                  area.geom())) {
                    _writer->writeTriple(
                        s, osm2ttl::ttl::constants::IRI_OGC_CONTAINS,
                        _writer->generateIRI(
                            osm2ttl::ttl::constants::NAMESPACE__OSM_NODE,
                            entryId));
                  }
                  break;
                  // Handle way
                case 1:
                  if (entryId != area.objId()) {
                    if (boost::geometry::covered_by(std::get<1>(geometry),
                                                    area.geom())) {
                      _writer->writeTriple(
                          s, osm2ttl::ttl::constants::IRI_OGC_CONTAINS,
                          _writer->generateIRI(
                              osm2ttl::ttl::constants::NAMESPACE__OSM_WAY,
                              entryId));
                    }
                  }
                  break;
                  // Handle area
                case 2:
                  if (entryId != area.objId()) {
                    if (boost::geometry::covered_by(std::get<2>(geometry),
                                                    area.geom())) {
                      _writer->writeTriple(
                          s, osm2ttl::ttl::constants::IRI_OGC_CONTAINS,
                          _writer->generateIRI(
                              osm2ttl::ttl::constants::NAMESPACE__OSM_RELATION,
                              entryId));
                    }
                  }
                  break;
              }
            }
          }
        }
        progressBar.update(entryCount++);
      }
    }
  }
  progressBar.done();
  std::cerr << " ... done" << std::endl;
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;