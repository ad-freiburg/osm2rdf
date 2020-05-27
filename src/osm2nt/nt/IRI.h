// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_IRI_H_
#define OSM2NT_NT_IRI_H_

#include <string>

#include "osmium/osm/object.hpp"

#include "osm2nt/nt/Object.h"
#include "osm2nt/nt/Predicate.h"
#include "osm2nt/nt/Subject.h"

namespace osm2nt {
namespace nt {

struct IRI : public Subject, public Predicate, public Object {
 public:
  explicit IRI(const std::string& prefix, const osmium::OSMObject& object);
  explicit IRI(const std::string& prefix, const std::string& s);
  std::string toString() const;
  std::string prefix;
  std::string value;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_IRI_H_
