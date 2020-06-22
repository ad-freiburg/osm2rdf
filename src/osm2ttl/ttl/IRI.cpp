// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/IRI.h"

#include <cctype>
#include <sstream>
#include <string>

#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/object.hpp"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osmium::NodeRef& n) {
  _prefix = prefix;
  std::stringstream tmp;
  tmp << n.positive_ref();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osmium::OSMObject& o) {
  _prefix = prefix;
  std::stringstream tmp;
  tmp << o.positive_id();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix,
                     const osmium::RelationMember& m) {
  _prefix = prefix;
  std::stringstream tmp;
  tmp << m.positive_ref();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const std::string& s) {
  _prefix = prefix;
  _value = s;
  // Trim strings
  _value.erase(_value.begin(),
               std::find_if(_value.begin(), _value.end(), [](char c) {
                 return std::isspace(c) != 0;
  }));
  _value.erase(std::find_if(_value.rbegin(), _value.rend(), [](char c) {
    return std::isspace(c) != 0;
  }).base(), _value.end());
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::IRI::prefix() const {
  return _prefix;
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::IRI::value() const {
  return _value;
}
