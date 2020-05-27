// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_LANGTAG_H_
#define OSM2NT_NT_LANGTAG_H_

#include <string>

namespace osm2nt {
namespace nt {

struct LangTag {
 public:
  explicit LangTag(const std::string& s);
  std::string toString() const;
 protected:
  std::string value;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_LANGTAG_H_


