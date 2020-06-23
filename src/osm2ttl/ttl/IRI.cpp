// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/IRI.h"

#include <cctype>
#include <iterator>
#include <sstream>
#include <string>
#include <iostream>

#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/object.hpp"
#include "osmium/osm/relation.hpp"

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osmium::NodeRef& n)
  : _prefix(prefix) {
  std::stringstream tmp;
  tmp << n.positive_ref();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osmium::OSMObject& o)
  : _prefix(prefix) {
  std::stringstream tmp;
  tmp << o.positive_id();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix,
                       const osmium::RelationMember& m)
  : _prefix(prefix) {
  std::stringstream tmp;
  tmp << m.positive_ref();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osm2ttl::osm::Area& a)
  : _prefix(prefix) {
  std::stringstream tmp;
  tmp << a.id();
  _value = tmp.str();
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const std::string& s)
  : _prefix(prefix) {
  auto begin = std::find_if(s.begin(), s.end(), [](int c) {
    return std::isspace(c) == 0;
  });
  auto end = std::find_if(s.rbegin(), s.rend(), [](int c) {
    return std::isspace(c) == 0;
  });
  _value = s.substr(begin - s.begin(), std::distance(begin, end.base()));
  // Trim strings
  if (s != _value) {
    std::cout << "s:" << s << " -> "<< _value << "\n";
  }
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::IRI::prefix() const noexcept {
  return _prefix;
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::IRI::value() const noexcept {
  return _value;
}
