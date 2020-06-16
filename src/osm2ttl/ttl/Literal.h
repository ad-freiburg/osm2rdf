// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2TTL_TTL_LITERAL_H_
#define OSM2TTL_TTL_LITERAL_H_

#include <string>

#include "osmium/osm/box.hpp"

#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/LangTag.h"

namespace osm2ttl {
namespace ttl {

struct Literal {
 public:
  explicit Literal(const std::string& s);
  explicit Literal(const osmium::Box& b);
  Literal(const std::string& s, const osm2ttl::ttl::IRI& i);
  Literal(const std::string& s, const osm2ttl::ttl::LangTag& l);
  std::string value() const;
  std::optional<osm2ttl::ttl::IRI> iri() const;
  std::optional<osm2ttl::ttl::LangTag> langTag() const;
 protected:
  std::optional<osm2ttl::ttl::IRI> _iri;
  std::optional<osm2ttl::ttl::LangTag> _langTag;
  std::string _value;
};

}  // namespace ttl
}  // namespace osm2ttl

#endif  // OSM2TTL_TTL_LITERAL_H_

