// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/OsmiumHandler.h"

#include "osm2ttl/osm/DumpHandler.h"
#include "osm2ttl/osm/GeometryHandler.h"

// ____________________________________________________________________________
template <typename W>
osm2ttl::osm::OsmiumHandler<W>::OsmiumHandler(
    const osm2ttl::config::Config& config, osm2ttl::ttl::Writer<W>* writer)
    : _config(config), _dumpHandler(osm2ttl::osm::DumpHandler<W>(config, writer)), _geometryHandler(osm2ttl::osm::GeometryHandler<W>(config, writer)) {}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::area(const osmium::Area& area) {
  if (_config.noAreaDump) {
    return;
  }
  const auto& a = osm2ttl::osm::Area(area);
  if (!_config.noDump) {
    _dumpHandler.area(a);
  }
  if (!_config.noContains) {
    if (!a.hasName()) {
      return;
    }
    _geometryHandler.area(a);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::node(const osmium::Node& node) {
  if (_config.noNodeDump) {
    return;
  }
  const auto& n = osm2ttl::osm::Node(node);
  if (node.tags().empty()) {
    return;
  }
  if (!_config.noDump) {
    _dumpHandler.node(n);
  }
  if (!_config.noContains) {
    _geometryHandler.node(n);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::relation(const osmium::Relation& relation) {
  if (_config.noRelationDump) {
    return;
  }
  const auto& r = osm2ttl::osm::Relation(relation);
  if (relation.tags().empty()) {
    return;
  }
  if (!_config.noDump) {
    _dumpHandler.relation(r);
  }
}

// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::way(const osmium::Way& way) {
  if (_config.noWayDump) {
    return;
  }
  const auto& w = osm2ttl::osm::Way(way);
  if (way.tags().empty()) {
    return;
  }
  if (!_config.noDump) {
    _dumpHandler.way(w);
  }
  if (!_config.noContains) {
    _geometryHandler.way(w);
  }
}
// ____________________________________________________________________________
template <typename W>
void osm2ttl::osm::OsmiumHandler<W>::calculateRelations() {
  if (_config.noContains) {
    return;
  }
  _geometryHandler.calculateRelations();
}

// ____________________________________________________________________________
template class osm2ttl::osm::OsmiumHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::OsmiumHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::OsmiumHandler<osm2ttl::ttl::format::QLEVER>;