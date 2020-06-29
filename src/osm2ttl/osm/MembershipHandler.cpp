// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/MembershipHandler.h"

#include <iostream>

#include "osmium/index/multimap/sparse_file_array.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/CacheFile.h"

// ____________________________________________________________________________
osm2ttl::osm::MembershipHandler::MembershipHandler(
  const osm2ttl::config::Config& config) : _config(config),
  _node2relationFile(config.getTempPath("osmium", "n2r.cache")),
  _node2wayFile(config.getTempPath("osmium", "n2w.cache")),
  _relation2areaFile(config.getTempPath("osmium", "r2a.cache")),
  _relation2relationFile(config.getTempPath("osmium", "r2r.cache")),
  _way2areaFile(config.getTempPath("osmium", "w2a.cache")),
  _way2relationFile(config.getTempPath("osmium", "w2r.cache")) {
  _node2wayIndex =
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>(
      _node2wayFile.fileDescriptor());

  _node2relationIndex =
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>(
      _node2relationFile.fileDescriptor());

  _way2relationIndex =
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>(
      _way2relationFile.fileDescriptor());

  _relation2relationIndex =
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>(
      _relation2relationFile.fileDescriptor());

  _way2areaIndex =
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>(
      _way2areaFile.fileDescriptor());

  _relation2areaIndex =
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>(
      _relation2areaFile.fileDescriptor());
}

// ____________________________________________________________________________
osm2ttl::osm::MembershipHandler::~MembershipHandler() {
  _node2relationFile.close();
  _node2wayFile.close();
  _relation2areaFile.close();
  _relation2relationFile.close();
  _way2areaFile.close();
  _way2relationFile.close();
  _node2relationFile.remove();
  _node2wayFile.remove();
  _relation2areaFile.remove();
  _relation2relationFile.remove();
  _way2areaFile.remove();
  _way2relationFile.remove();
}

// ____________________________________________________________________________
void osm2ttl::osm::MembershipHandler::sort() {
  std::cerr << " n2r " << _node2relationIndex.size() << " elements ... ";
  _node2relationIndex.sort();
  std::cerr << "done\n" << " n2w " << _node2relationIndex.size()
    << " elements ... ";
  _node2wayIndex.sort();
  std::cerr << "done\n" << " r2a " << _relation2areaIndex.size()
    << " elements ... ";
  _relation2areaIndex.sort();
  std::cerr << "done\n" << " r2r " << _relation2relationIndex.size()
    << " elements ... ";
  _relation2relationIndex.sort();
  std::cerr << "done\n" << " w2a " << _way2areaIndex.size()
    << " elements ... ";
  _way2areaIndex.sort();
  std::cerr << "done\n" << " w2r " << _way2relationIndex.size()
    << " elements ... ";
  _way2relationIndex.sort();
  std::cerr << "done" << std::endl;
}

// ____________________________________________________________________________
bool osm2ttl::osm::MembershipHandler::isNodeMemberOfAnyWay(
  const osmium::Node& node) {
  auto pos = _node2wayIndex.get_all(node.id());
  return pos.first != pos.second;
}

// ____________________________________________________________________________
bool osm2ttl::osm::MembershipHandler::isNodeMemberOfAnyRelation(
  const osmium::Node& node) {
  auto pos = _node2relationIndex.get_all(node.id());
  return pos.first != pos.second;
}

// ____________________________________________________________________________
bool osm2ttl::osm::MembershipHandler::isRelationMemberOfAnyRelation(
  const osmium::Relation& relation) {
  auto pos = _relation2relationIndex.get_all(relation.id());
  return pos.first != pos.second;
}

// ____________________________________________________________________________
bool osm2ttl::osm::MembershipHandler::isRelationMemberOfAnyArea(
  const osmium::Relation& relation) {
  auto pos = _relation2areaIndex.get_all(relation.id());
  return pos.first != pos.second;
}

// ____________________________________________________________________________
bool osm2ttl::osm::MembershipHandler::isWayMemberOfAnyRelation(
  const osmium::Way& way) {
  auto pos = _way2relationIndex.get_all(way.id());
  return pos.first != pos.second;
}

// ____________________________________________________________________________
bool osm2ttl::osm::MembershipHandler::isWayMemberOfAnyArea(
  const osmium::Way& way) {
  auto pos = _way2areaIndex.get_all(way.id());
  return pos.first != pos.second;
}

// ____________________________________________________________________________
void osm2ttl::osm::MembershipHandler::area(const osmium::Area& area) {
  if (area.from_way()) {
    _way2areaIndex.set(area.orig_id(), area.positive_id());
  } else {
    _relation2areaIndex.set(area.orig_id(), area.positive_id());
  }
}
// ____________________________________________________________________________
void osm2ttl::osm::MembershipHandler::relation(
  const osmium::Relation& relation) {
  for (const auto& member : relation.members()) {
    switch (member.type()) {
      case osmium::item_type::node:
        _node2relationIndex.set(member.positive_ref(), relation.positive_id());
        break;
      case osmium::item_type::way:
        _way2relationIndex.set(member.positive_ref(), relation.positive_id());
        break;
      case osmium::item_type::relation:
        _relation2relationIndex.set(member.positive_ref(),
                                    relation.positive_id());
        break;
      default:
        break;
    }
  }
}
// ____________________________________________________________________________
void osm2ttl::osm::MembershipHandler::way(const osmium::Way& way) {
  for (const auto& nodeRef : way.nodes()) {
    _node2wayIndex.set(nodeRef.positive_ref(), way.positive_id());
  }
}
