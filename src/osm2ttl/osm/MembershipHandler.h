// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_OSM_MEMBERSHIPHANDLER_H_
#define OSM2TTL_OSM_MEMBERSHIPHANDLER_H_

#include <filesystem>

#include "osmium/index/multimap/sparse_file_array.hpp"
#include "osmium/handler.hpp"
#include "osmium/handler/object_relations.hpp"
#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"
#include "osm2ttl/osm/CacheFile.h"

namespace osm2ttl {
namespace osm {

// Partially based on osmium::handler::ObjectRelations
class MembershipHandler : public osmium::handler::Handler {
 public:
  explicit MembershipHandler(const osm2ttl::config::Config& config);
  ~MembershipHandler();

  void sort();

  bool isNodeMemberOfAnyRelation(const osmium::Node& node);
  bool isNodeMemberOfAnyWay(const osmium::Node& node);
  bool isRelationMemberOfAnyArea(const osmium::Relation& relation);
  bool isRelationMemberOfAnyRelation(const osmium::Relation& relation);
  bool isWayMemberOfAnyArea(const osmium::Way& way);
  bool isWayMemberOfAnyRelation(const osmium::Way& way);

  // Handler interface
  void area(const osmium::Area& area);
  void relation(const osmium::Relation& relation);
  void way(const osmium::Way& way);

 protected:
  osm2ttl::config::Config _config;

  osm2ttl::osm::CacheFile _node2relationFile;
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>
    _node2relationIndex;
  osm2ttl::osm::CacheFile _node2wayFile;
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>
    _node2wayIndex;

  osm2ttl::osm::CacheFile _relation2areaFile;
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>
    _relation2areaIndex;
  osm2ttl::osm::CacheFile _relation2relationFile;
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>
    _relation2relationIndex;

  osm2ttl::osm::CacheFile _way2areaFile;
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>
    _way2areaIndex;
  osm2ttl::osm::CacheFile _way2relationFile;
  osmium::index::multimap::SparseFileArray<
    osmium::unsigned_object_id_type, osmium::unsigned_object_id_type>
    _way2relationIndex;
};

}  // namespace osm
}  // namespace osm2ttl

#endif  // OSM2TTL_OSM_MEMBERSHIPHANDLER_H_
