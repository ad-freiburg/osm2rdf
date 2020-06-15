// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_IRI_H_
#define OSM2NT_NT_IRI_H_

#include <string>

#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/object.hpp"
#include "osmium/osm/relation.hpp"

namespace osm2nt {
namespace nt {

struct IRI {
 public:
  IRI(const std::string& prefix, const std::string& s);
  IRI(const std::string& prefix, const osmium::NodeRef& n);
  IRI(const std::string& prefix, const osmium::OSMObject& o);
  IRI(const std::string& prefix, const osmium::RelationMember& m);
  std::string prefix() const;
  std::string value() const;
 protected:
  std::string _prefix;
  std::string _value;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_IRI_H_
