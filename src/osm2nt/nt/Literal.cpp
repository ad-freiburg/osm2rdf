// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/Literal.h"

#include <string>
#include <sstream>

#include "osm2nt/nt/IRI.h"
#include "osm2nt/nt/LangTag.h"

// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const std::string& s) {
  std::stringstream tmp;
  tmp << "\"" << s << "\"";
  value = tmp.str();
  value = s;
  iri = nullptr;
  langTag = nullptr;
}

// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const std::string &s, const osm2nt::nt::IRI* i)
  : osm2nt::nt::Literal::Literal(s) {
  iri = i;
}

// ____________________________________________________________________________
osm2nt::nt::Literal::Literal(const std::string &s, const osm2nt::nt::LangTag* l)
  : osm2nt::nt::Literal::Literal(s) {
  langTag = l;
}

// ____________________________________________________________________________
std::string osm2nt::nt::Literal::toString() const {
  std::stringstream tmp;
  tmp << "\"";
  // Escape value
  for (size_t pos = 0; pos < value.size(); ++pos) {
    switch (value[pos]) {
      case '\\':
        tmp << "\\\\";
        break;
      case '\n':
        tmp << "\\n";
        break;
      case '"':
        tmp << "\\\"";
        break;
      case '\r':
        tmp << "\\r";
        break;
      default:
        tmp << value[pos];
    }
  }
  tmp << "\"";
  if (iri != nullptr) {
    tmp << "^^" << iri->toString();
  }
  if (langTag != nullptr) {
    tmp << "@" << langTag->toString();
  }
  return tmp.str();
}
