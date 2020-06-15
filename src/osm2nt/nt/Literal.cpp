// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/Literal.h"

#include <string>

#include "osmium/osm/box.hpp"

#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/LangTag.h"

// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const std::string& s) {
  _value = s;
  _iri = std::nullopt;
  _langTag = std::nullopt;
}


// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const osmium::Box& b)
  : osm2nt::nt::Literal::Literal("POLYGON(("
      +std::to_string(b.bottom_left().lon_without_check())+" "
      +std::to_string(b.top_right().lat_without_check())+","
      +std::to_string(b.top_right().lon_without_check())+" "
      +std::to_string(b.top_right().lat_without_check())+","
      +std::to_string(b.top_right().lon_without_check())+" "
      +std::to_string(b.bottom_left().lat_without_check())+","
      +std::to_string(b.bottom_left().lon_without_check())+" "
      +std::to_string(b.bottom_left().lat_without_check())+"))") {}

// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const std::string &s, const osm2nt::nt::IRI& i)
  : osm2nt::nt::Literal::Literal(s) {
  _iri = i;
}

// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const std::string &s, const osm2nt::nt::LangTag& l)
  : osm2nt::nt::Literal::Literal(s) {
  _langTag = l;
}

// ____________________________________________________________________________
std::string osm2nt::nt::Literal::value() const {
  return _value;
}

// ____________________________________________________________________________
std::optional<osm2nt::nt::IRI> osm2nt::nt::Literal::iri() const {
  return _iri;
}

// ____________________________________________________________________________
std::optional<osm2nt::nt::LangTag> osm2nt::nt::Literal::langTag() const {
  return _langTag;
}
