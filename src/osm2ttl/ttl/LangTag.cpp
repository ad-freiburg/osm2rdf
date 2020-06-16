// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/LangTag.h"

#include <string>

// ____________________________________________________________________________
osm2ttl::ttl::LangTag::LangTag(const std::string& s) {
  _value = s;
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::LangTag::value() const {
  return _value;
}
