// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/GeometryHandler.h"

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
osm2ttl::osm::GeometryHandler<W>::GeometryHandler(const osm2ttl::config::Config& config,
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
osm2ttl::osm::GeometryHandler<W>::~GeometryHandler() {
  _areasFile.close();
  _areasFile.remove();
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::area(const osmium::Area& area) {
  if (_sorted) {
    std::cerr << "Handling area: " << area.orig_id() << std::endl;
    osm2ttl::osm::Area a{area};
    std::string o = _writer->generateIRI(a.fromWay()?"osmway":"osmrel", a.objId());
    std::vector<SpatialValue> results;
    _spatialIndex.query(boost::geometry::index::covers(a.envelope()), std::back_inserter(results));

    for (auto [box, areaId] : results) {
      auto carea = _areas.get(areaId);
      if (boost::geometry::within(a.geom(), carea.geom())) {
        std::string s = _writer->generateIRI(carea.fromWay()?"osmway":"osmrel", carea.objId());
        _writer->writeTriple(s, "ogc:contains", o);
      }
    }
  } else {
    osm2ttl::osm::Area a(area);
    if (a.tagAdministrationLevel() > 0) {
      _areas.set(a.id(), a);
      _spatialIndex.insert(std::make_pair(a.envelope(), a.id()));
    }
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::node(const osmium::Node& node) {
  if(!_sorted) {
    return;
  }
  auto n = osm2ttl::osm::Node(node);
  // Skip unnamed nodes for now
  if (n.tags().count("name") < 1) {
    return;
  }
  auto posNodeId = n.id();
  std::string o = _writer->generateIRI("osmnode", posNodeId);
  auto location = n.geom();
  std::vector<SpatialValue> results;
  _spatialIndex.query(boost::geometry::index::covers(location), std::back_inserter(results));

  for (auto [box, areaId] : results) {
    auto carea = _areas.get(areaId);
    if (!boost::geometry::within(location, carea.geom())) {
      continue;
    }
    _writer->writeTriple(_writer->generateIRI(carea.fromWay()?"osmway":"osmrel", carea.objId()), "ogc:contains", o);
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::relation(const osmium::Relation& relation) {
  if (_sorted) {
    return;
  }
  for (const auto& member : relation.members()) {
    if (member.type() == osmium::item_type::node && (strcmp(member.role(), "inner") == 0 ||  strcmp(member.role(), "outer") == 0)) {
      _locationRelationMap.insert({member.positive_ref(), relation.positive_id()});
    }
    if (member.type() == osmium::item_type::way) {
      _wayRelationMap.insert({member.positive_ref(), relation.positive_id()});
    }
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::way(const osmium::Way& way) {
  if (_sorted) {
    return;
  }
  for (const auto& node : way.nodes()) {
    _wayLocationMap.insert({node.positive_ref(), way.positive_id()});
  }
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::GeometryHandler<W>::prepareLookup() {
  {
    std::cerr << " Sorting " << _areas.size() << " areas ... ";
    _areas.sort();
    std::cerr << "done" << std::endl;
  }
  {
    std::cerr << " Update node relation map from ways for " << _wayLocationMap.size() << " entries ... " << std::endl;
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
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::GeometryHandler<osm2ttl::ttl::format::QLEVER>;