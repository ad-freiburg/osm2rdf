// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

#include <iostream>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "boost/thread.hpp"
#include "boost/geometry.hpp"
#include "boost/geometry/index/rtree.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/util/progress_bar.hpp"

// ____________________________________________________________________________
template<typename W>
osm2ttl::osm::GeometryHandler<W>::GeometryHandler(const osm2ttl::config::Config& config,
                                       osm2ttl::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer),
_areasFile(config.getTempPath("osm2ttl", "areas.cache")),
_waysFile(config.getTempPath("osm2ttl", "ways.cache")){
  _areas = osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osm2ttl::osm::Area>(
      _areasFile.fileDescriptor());
  _ways = osmium::index::map::SparseFileArray<
          osmium::unsigned_object_id_type, osm2ttl::osm::Way>(
      _waysFile.fileDescriptor());
}

// ____________________________________________________________________________
template<typename W>
osm2ttl::osm::GeometryHandler<W>::~GeometryHandler() {
  _areasFile.close();
  _areasFile.remove();
  _waysFile.close();
  _waysFile.remove();
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::area(const osmium::Area& area) {
  osm2ttl::osm::Area a = osm2ttl::osm::Area(area);
  _areas.set(a.id(), a);
  _spatialAreaIndex.insert(std::make_pair(a.envelope(), a.id()));
  _spatialIndex.insert(std::make_pair(a.envelope(), std::make_pair(a.id(), 2)));
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::node(const osmium::Node& node) {
  osm2ttl::osm::Node n = osm2ttl::osm::Node(node);
  if (n.tags().count("name") < 1) {
    return;
  }
  _spatialNodeIndex.insert(std::make_pair(n.geom(), n.id()));
  _spatialIndex.insert(std::make_pair(n.envelope(), std::make_pair(n.id(), 0)));
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::way(const osmium::Way& way) {
  osm2ttl::osm::Way w = osm2ttl::osm::Way(way);
  _ways.set(w.id(), w);
  _spatialWayIndex.insert(std::make_pair(w.envelope(), w.id()));
  _spatialIndex.insert(std::make_pair(w.envelope(), std::make_pair(w.id(), 1)));
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::prepareLookup() {
  {
    std::cerr << " Sorting " << _areas.size() << " areas ... " << std::flush;
    _areas.sort();
    std::cerr << "done" << std::endl;
    std::cerr << " Sorting " << _ways.size() << " ways ... " << std::flush;
    _ways.sort();
    std::cerr << "done" << std::endl;
    std::cerr << " Packing node tree with  " << _spatialNodeIndex.size() << " nodes ... " << std::flush;
    SpatialNodeIndex tmpNodeIndex(_spatialNodeIndex.begin(), _spatialNodeIndex.end());
    _spatialNodeIndex = std::move(tmpNodeIndex);
    std::cerr << "done" << std::endl;
    std::cerr << " Packing way tree with  " << _spatialWayIndex.size() << " ways ... " << std::flush;
    SpatialWayIndex tmpWayIndex(_spatialWayIndex.begin(), _spatialWayIndex.end());
    _spatialWayIndex = std::move(tmpWayIndex);
    std::cerr << "done" << std::endl;
    std::cerr << " Packing area tree with  " << _spatialAreaIndex.size() << " areas ... " << std::flush;
    SpatialAreaIndex tmpAreaIndex(_spatialAreaIndex.begin(), _spatialAreaIndex.end());
    _spatialAreaIndex = std::move(tmpAreaIndex);
    std::cerr << "done" << std::endl;
    std::cerr << " Packing combined tree with  " << _spatialIndex.size() << " entries ... " << std::flush;
    SpatialIndex tmpIndex(_spatialIndex.begin(), _spatialIndex.end());
    _spatialIndex = std::move(tmpIndex);
    std::cerr << "done" << std::endl;
  }
  _sorted = true;
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::lookup() {
  if (!_sorted) {
    throw std::runtime_error("GeometryHandler was not prepared for lookup!");
  }
  std::cerr << " Contains relations for " << _spatialAreaIndex.size() << " areas, " << _spatialWayIndex.size() << " ways, and " << _spatialNodeIndex.size() << " nodes ... " << std::endl;
  osmium::ProgressBar progressBar{_areas.size(), true};
  size_t entryCount = 0;
  progressBar.update(entryCount);

  for (auto [areaId, area] : _areas) {
    area.geom();
    if (area.tagAdministrationLevel() > 0) {
      std::string s = _writer->generateIRI(area.fromWay()?"osmway":"osmrel", area.objId());
      auto start1 = std::chrono::high_resolution_clock::now();
      for (auto nit = _spatialNodeIndex.qbegin(boost::geometry::index::covered_by(area.geom())); nit != _spatialNodeIndex.qend(); nit++) {
        _writer->writeTriple(s, "ogc:contains", _writer->generateIRI("osmnode", nit->second));
      }
      for (auto wit = _spatialWayIndex.qbegin(boost::geometry::index::covered_by(area.envelope())); wit != _spatialWayIndex.qend(); wit++) {
        // Skip self containment
        if (wit->second == area.objId()) {
          continue;
        }
        auto oway = _ways.get(wit->second);
        if (boost::geometry::covered_by(oway.geom(), area.geom())) {
          _writer->writeTriple(s, "ogc:contains", _writer->generateIRI("osmway", oway.id()));
        }
      }
      for (auto ait = _spatialAreaIndex.qbegin(boost::geometry::index::covered_by(area.envelope())); ait != _spatialAreaIndex.qend(); ait++) {
        // Skip self containment
        if (ait->second == areaId) {
          continue;
        }
        auto oarea = _areas.get(ait->second);
        if (boost::geometry::covered_by(oarea.geom(), area.geom())) {
          _writer->writeTriple(s, "ogc:contains", _writer->generateIRI(oarea.fromWay()?"osmway":"osmrel", oarea.objId()));
        }
      }
      auto stop1 = std::chrono::high_resolution_clock::now();
      auto start2 = std::chrono::high_resolution_clock::now();
      for (auto it = _spatialIndex.qbegin(boost::geometry::index::covered_by(area.envelope())); it != _spatialIndex.qend(); it++) {
        auto entryId = it->second.first;
        switch(it->second.second) {
          // Handle node
          case 0:
            if (boost::geometry::covered_by(it->first.min_corner(), area.geom())) {
              _writer->writeTriple(s, "ogc:contains", _writer->generateIRI("osmnode", entryId));
            }
            break;
            // Handle way
          case 1:
            if (entryId != area.objId()) {
              auto oway = _ways.get(entryId);
              if (boost::geometry::covered_by(oway.geom(), area.geom())) {
                _writer->writeTriple(s, "ogc:contains", _writer->generateIRI("osmway", oway.id()));
              }
            }
            break;
            // Handle area
          case 2:
            if (entryId != areaId) {
              auto oarea = _areas.get(entryId);
              if (boost::geometry::covered_by(oarea.geom(), area.geom())) {
                _writer->writeTriple(s, "ogc:contains", _writer->generateIRI(oarea.fromWay()?"osmway":"osmrel", oarea.objId()));
              }
            }
            break;
        }
      }
      auto stop2 = std::chrono::high_resolution_clock::now();
      auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(stop1 - start1);
      auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
      std::cerr << "\n" << "D1: " << duration1.count() << " microseconds" << "\n" << "D2: " << duration2.count() << " microseconds" << std::endl;

    }
    progressBar.update(entryCount++);
  }
  progressBar.done();
  std::cerr << " ... done" << std::endl;
}

// ____________________________________________________________________________
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;