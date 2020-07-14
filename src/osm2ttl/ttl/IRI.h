// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_IRI_H_
#define OSM2TTL_TTL_IRI_H_

#include <string>

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/RelationMember.h"
#include "osm2ttl/osm/Way.h"

namespace osm2ttl {
namespace ttl {

struct IRI {
 public:
  IRI() = default;
  IRI(const std::string& prefix, const std::string& s);
  IRI(const std::string& prefix, const osm2ttl::osm::Area& a);
  IRI(const std::string& prefix, const osm2ttl::osm::Node& n);
  IRI(const std::string& prefix, const osm2ttl::osm::Relation& r);
  IRI(const std::string& prefix, const osm2ttl::osm::RelationMember& m);
  IRI(const std::string& prefix, const osm2ttl::osm::Way& w);
  std::string prefix() const noexcept;
  std::string value() const noexcept;
 protected:
  std::string _prefix;
  std::string _value;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_IRI_H_
