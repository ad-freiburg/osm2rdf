// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_LANGTAG_H_
#define OSM2TTL_TTL_LANGTAG_H_

#include <string>

namespace osm2ttl {
namespace ttl {

struct LangTag {
 public:
  explicit LangTag(const std::string& s);
  std::string value() const;
 protected:
  std::string _value;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_LANGTAG_H_


