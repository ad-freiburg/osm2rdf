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
  this->prefix = prefix;
  std::stringstream tmp;
  tmp << n.positive_ref();
  value = tmp.str();
}

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix, const osmium::OSMObject& o) {
  this->prefix = prefix;
  std::stringstream tmp;
  tmp << o.positive_id();
  value = tmp.str();
}

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix,
                     const osmium::RelationMember& m) {
  this->prefix = prefix;
  std::stringstream tmp;
  tmp << m.positive_ref();
  value = tmp.str();
}

// ____________________________________________________________________________
osm2nt::nt::IRI::IRI(const std::string& prefix, const std::string& s) {
  this->prefix = prefix;
  value = s;
}

// ____________________________________________________________________________
std::string osm2nt::nt::IRI::toString() const {
  std::stringstream tmp;
  tmp << "<" << prefix;
  for (size_t pos = 0; pos < value.size(); ++pos) {
    switch (value[pos]) {
      case '|':
        tmp << "%7C";
        break;
      default:
        tmp << value[pos];
    }
  }
  tmp << ">";
  return tmp.str();
}
