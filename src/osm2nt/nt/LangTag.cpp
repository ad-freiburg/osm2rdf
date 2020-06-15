// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/LangTag.h"

#include <string>

// ____________________________________________________________________________
osm2nt::nt::LangTag::LangTag(const std::string& s) {
  _value = s;
}

// ____________________________________________________________________________
std::string osm2nt::nt::LangTag::value() const {
  return _value;
}
