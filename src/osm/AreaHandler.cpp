// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/AreaHandler.h"

#include <boost/geometry.hpp>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/Area.h"
#include "osm2ttl/ttl/Writer.h"
#include "osmium/index/map/sparse_file_array.hpp"
#include "osmium/util/progress_bar.hpp"

// ____________________________________________________________________________
template<typename W>
osm2ttl::osm::AreaHandler<W>::AreaHandler(const osm2ttl::config::Config& config,
                                       osm2ttl::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer),
_areasFile(config.getTempPath("osm2ttl", "areas.cache")) {
  _areas =
  osmium::index::map::SparseFileArray<
    osmium::unsigned_object_id_type, osm2ttl::osm::Area>(
      _areasFile.fileDescriptor());
}

// ____________________________________________________________________________
template<typename W>
osm2ttl::osm::AreaHandler<W>::~AreaHandler() {
  _areasFile.close();
  _areasFile.remove();
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::AreaHandler<W>::area(const osmium::Area& area) {
  if (_sorted) {
    std::cerr << "Handling area: " << area.orig_id() << std::endl;
    osm2ttl::osm::Area a{area};
    std::string o = _writer->generateIRI(a.fromWay()?"osmway":"osmrel", a.objId());
    std::unordered_set<uint64_t> seen;

    auto envelope = a.envelope();
    auto reducedMinCorner = reducedCoordinates(envelope.min_corner());
    auto reducedMaxCorner = reducedCoordinates(envelope.max_corner());
    for (uint8_t sx = reducedMinCorner.first; sx <= reducedMaxCorner.first; sx++) {
      for (uint8_t sy = reducedMinCorner.second; sy <= reducedMaxCorner.second; sy++) {
        for (auto [ccomplete, carea] : _stacks[stackIndex(sx, sy)]) {
          if (seen.count(carea->id()) > 0) {
            continue;
          }
          seen.insert(carea->id());
          if (carea->objId() == a.objId()) {
            continue;
          }
          if (boost::geometry::within(a.geom(), carea->geom())) {
            std::string s = _writer->generateIRI(carea->fromWay()?"osmway":"osmrel", carea->objId());
            _writer->writeTriple(s, "ogc:contains", o);
          }
        }
      }
    }
  } else {
    osm2ttl::osm::Area a(area);
    if (a.tagAdministrationLevel() > 0) {
      _areas.set(a.id(), a);
    }
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::AreaHandler<W>::node(const osmium::Node& node) {
  if(!_sorted) {
    return;
  }
  auto posNodeId = node.positive_id();
  std::string o = _writer->generateIRI("osmnode", posNodeId);
  auto location = osm2ttl::osm::Node(node).geom();
  auto reducedLocation = reducedCoordinates(location);
  for (auto [ccomplete, carea] : _stacks[stackIndex(reducedLocation.first, reducedLocation.second)]) {
    auto areaId = carea->objId();
    bool matched = false;
    if (!matched && ccomplete) {
      matched = true;
    }
    if (!matched) {
      auto nodeKeyRange = _locationRelationMap.equal_range(posNodeId);
      for (auto it = nodeKeyRange.first; !matched && it != nodeKeyRange.second; it++) {
        if (it->second == areaId) {
          matched = true;
        }
      }
    }
    if (!matched && boost::geometry::within(location, carea->envelope())) {
      matched = true;
      if (matched && !boost::geometry::within(location, carea->geom())) {
        matched = false;
      }
    }
    if (matched) {
      _writer->writeTriple(_writer->generateIRI(carea->fromWay()?"osmway":"osmrel", areaId), "ogc:contains", o);
    }
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::AreaHandler<W>::relation(const osmium::Relation& relation) {
  if (_sorted) {
    return;
  }
  for (const auto& member : relation.members()) {
    if (member.type() == osmium::item_type::node && (member.role() == "inner" || member.role() == "outer")) {
      _locationRelationMap.insert({member.positive_ref(), relation.positive_id()});
    }
    if (member.type() == osmium::item_type::way) {
      _wayRelationMap.insert({member.positive_ref(), relation.positive_id()});
    }
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::AreaHandler<W>::way(const osmium::Way& way) {
  if (_sorted) {
    return;
  }
  for (const auto& node : way.nodes()) {
    _wayLocationMap.insert({node.positive_ref(), way.positive_id()});
  }
}

// ____________________________________________________________________________
template<typename W>
constexpr uint16_t osm2ttl::osm::AreaHandler<W>::stackIndex(uint8_t x, uint8_t y) const {
  return 361 * x + y;
}

// ____________________________________________________________________________
template<typename W>
std::pair<uint8_t, uint8_t> osm2ttl::osm::AreaHandler<W>::reducedCoordinates(const osm2ttl::geometry::Location& location) const {
  uint8_t nx = (180 + int(location.x())) / _xFactor;
  uint8_t ny = (90 + int(location.y())) / _yFactor;
  return std::make_pair(nx, ny);
}
// ____________________________________________________________________________
template<typename W>
osm2ttl::geometry::Polygon osm2ttl::osm::AreaHandler<W>::regionForIndex(uint8_t x, uint8_t y) const {
  return osm2ttl::geometry::Polygon({{{-180 + x, -90 + y},{-180 + x + 1, -90 + y + 1 }}});
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::AreaHandler<W>::sort() {
  {
    std::cerr << " Sorting " << _areas.size() << " areas ... ";
    _areas.sort();
    std::cerr << "done" << std::endl;
  }
  {
    std::cerr << " Generating area grid with " << _areas.size() << " areas ... " << std::endl;
    osmium::ProgressBar progressBar{_areas.size(), true};
    size_t areaIndex = 0;
    _stacks.resize(65536);
    progressBar.update(areaIndex);
    for (auto& [areaId, area] : _areas) {
      auto envelope = area.envelope();
      auto reducedMinCorner = reducedCoordinates(envelope.min_corner());
      auto reducedMaxCorner = reducedCoordinates(envelope.max_corner());
      for (uint8_t sx = reducedMinCorner.first; sx <= reducedMaxCorner.first; sx++) {
        for (uint8_t sy = reducedMinCorner.second; sy <= reducedMaxCorner.second; sy++) {
          _stacks[stackIndex(sx, sy)].emplace_back(boost::geometry::within(regionForIndex(sx, sy), area.geom()), &area);
        }
      }
      progressBar.update(areaIndex++);
    }
    progressBar.done();
    std::cerr << " ... done" << std::endl;
  }
  {
    std::cerr << " Update node relation map from ways for " << _wayLocationMap.size() << " entires ... " << std::endl;
    osmium::ProgressBar progressBar{_wayLocationMap.size(), true};
    size_t entryCount = 0;
    progressBar.update(entryCount);
    for (auto [nodeId, wayId] : _wayLocationMap) {
      _locationRelationMap.insert({nodeId, wayId});
      auto relationKeyRange = _wayRelationMap.equal_range(wayId);
      for (auto it = relationKeyRange.first; it != relationKeyRange.second; it++) {
        _locationRelationMap.insert({nodeId, it->second});
      }
      progressBar.update(entryCount++);
    }
    progressBar.done();
    _wayRelationMap.clear();
    _wayLocationMap.clear();
    std::cerr << " ... done" << std::endl;
  }

  _sorted = true;
}

// ____________________________________________________________________________
template class osm2ttl::osm::AreaHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::AreaHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::AreaHandler<osm2ttl::ttl::format::QLEVER>;