// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2nt/nt/IRI.h"

#include <sstream>
#include <ostream>

#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/object.hpp"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix, const osmium::NodeRef& n) {
  _prefix = prefix;
  std::stringstream tmp;
  tmp << n.positive_ref();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix, const osmium::OSMObject& o) {
  _prefix = prefix;
  std::stringstream tmp;
  tmp << o.positive_id();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix,
                     const osmium::RelationMember& m) {
  _prefix = prefix;
  std::stringstream tmp;
  tmp << m.positive_ref();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix, const std::string& s) {
  _prefix = prefix;
  _value = s;
}

// ____________________________________________________________________________
std::string osm2nt::nt::IRI::urlencode(const std::string& s) {
  std::stringstream tmp;
  for (size_t pos = 0; pos < s.size(); ++pos) {
    switch (s[pos]) {
      case ' ':
        tmp << "%20";
        break;
      case '\"':
        tmp << "%22";
        break;
      case '\'':
        tmp << "%24";
        break;
      case '%':
        tmp << "%25";
        break;
      case '&':
        tmp << "%26";
        break;
      case '(':
        tmp << "%28";
        break;
      case ')':
        tmp << "%29";
        break;
      case ',':
        tmp << "%2C";
        break;
      case '<':
        tmp << "%3C";
        break;
      case '>':
        tmp << "%3E";
        break;
      case '|':
        tmp << "%7C";
        break;
      default:
        tmp << s[pos];
    }
  }
  return tmp.str();
}

// ____________________________________________________________________________
std::string osm2nt::nt::IRI::toString() const {
  return "<" + _prefix + urlencode(_value) + ">";
}
