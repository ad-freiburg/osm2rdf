// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_LITERAL_H_
#define OSM2NT_NT_LITERAL_H_

#include <string>

#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/LangTag.h"
#include "osm2nt/nt/Object.h"

namespace osm2nt {
namespace nt {

struct Literal : public Object {
 public:
  explicit Literal(const std::string& s);
  Literal(const std::string& s, const osm2nt::nt::IRI* i);
  Literal(const std::string& s, const osm2nt::nt::LangTag* l);
  std::string toString() const;
 protected:
  const osm2nt::nt::IRI* iri;
  const osm2nt::nt::LangTag* langTag;
  std::string value;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_LITERAL_H_

