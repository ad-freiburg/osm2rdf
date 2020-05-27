// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/LangTag.h"

#include <string>
#include <sstream>

// ____________________________________________________________________________
osm2nt::nt::LangTag::LangTag(const std::string& s) {
  value = s;
}

// ____________________________________________________________________________
std::string osm2nt::nt::LangTag::toString() const {
  return value;
}

