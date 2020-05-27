// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_IRI_H_
#define OSM2NT_NT_IRI_H_

#include <string>

#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/object.hpp"
#include "osmium/osm/relation.hpp"

#include "osm2nt/nt/Object.h"
#include "osm2nt/nt/Predicate.h"
#include "osm2nt/nt/Subject.h"

namespace osm2nt {
namespace nt {

struct IRI : public Subject, public Predicate, public Object {
 public:
  explicit IRI(const std::string& prefix, const std::string& s);
  explicit IRI(const std::string& prefix, const osmium::NodeRef& n);
  explicit IRI(const std::string& prefix, const osmium::OSMObject& o);
  explicit IRI(const std::string& prefix, const osmium::RelationMember& m);
  std::string toString() const;
  std::string prefix;
  std::string value;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_IRI_H_
