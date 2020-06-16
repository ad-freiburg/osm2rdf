// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/Literal.h"

#include <string>

#include "osmium/osm/box.hpp"

#include "osm2ttl/ttl/IRI.h"
#include "osm2ttl/ttl/LangTag.h"

// ____________________________________________________________________________
osm2ttl::ttl::Literal::Literal(const std::string& s) {
  _value = s;
  _iri = std::nullopt;
  _langTag = std::nullopt;
}


// ____________________________________________________________________________
osm2ttl::ttl::Literal::Literal(const osmium::Box& b)
  : osm2ttl::ttl::Literal::Literal("POLYGON(("
      +std::to_string(b.bottom_left().lon_without_check())+" "
      +std::to_string(b.top_right().lat_without_check())+","
      +std::to_string(b.top_right().lon_without_check())+" "
      +std::to_string(b.top_right().lat_without_check())+","
      +std::to_string(b.top_right().lon_without_check())+" "
      +std::to_string(b.bottom_left().lat_without_check())+","
      +std::to_string(b.bottom_left().lon_without_check())+" "
      +std::to_string(b.bottom_left().lat_without_check())+"))") {}

// ____________________________________________________________________________
osm2ttl::ttl::Literal::Literal(const std::string &s, const osm2ttl::ttl::IRI& i)
  : osm2ttl::ttl::Literal::Literal(s) {
  _iri = i;
}

// ____________________________________________________________________________
osm2ttl::ttl::Literal::Literal(const std::string &s,
                               const osm2ttl::ttl::LangTag& l)
  : osm2ttl::ttl::Literal::Literal(s) {
  _langTag = l;
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::Literal::value() const {
  return _value;
}

// ____________________________________________________________________________
std::optional<osm2ttl::ttl::IRI> osm2ttl::ttl::Literal::iri() const {
  return _iri;
}

// ____________________________________________________________________________
std::optional<osm2ttl::ttl::LangTag> osm2ttl::ttl::Literal::langTag() const {
  return _langTag;
}
