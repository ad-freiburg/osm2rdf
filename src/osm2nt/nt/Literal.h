// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_NT_LITERAL_H_
#define OSM2NT_NT_LITERAL_H_

#include <string>

#include "osmium/osm/box.hpp"

#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/LangTag.h"

namespace osm2nt {
namespace nt {

struct Literal {
 public:
  explicit Literal(const std::string& s);
  explicit Literal(const osmium::Box& b);
  Literal(const std::string& s, const osm2nt::nt::IRI& i);
  Literal(const std::string& s, const osm2nt::nt::LangTag& l);
  std::string value() const;
  std::optional<osm2nt::nt::IRI> iri() const;
  std::optional<osm2nt::nt::LangTag> langTag() const;
 protected:
  std::optional<osm2nt::nt::IRI> _iri;
  std::optional<osm2nt::nt::LangTag> _langTag;
  std::string _value;
};

}  // namespace nt
}  // namespace osm2nt

#endif  // OSM2NT_NT_LITERAL_H_

