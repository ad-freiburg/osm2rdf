// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/IRI.h"

#include <cctype>
#include <iterator>
#include <string>
#include <iostream>

#include "osmium/osm/node_ref.hpp"
#include "osmium/osm/object.hpp"
#include "osmium/osm/relation.hpp"

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osmium::NodeRef& n)
  : _prefix(prefix),  _value(std::to_string(n.positive_ref())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osmium::OSMObject& o)
  : _prefix(prefix),  _value(std::to_string(o.positive_id())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix,
                       const osmium::RelationMember& m)
  : _prefix(prefix),  _value(std::to_string(m.positive_ref())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osm2ttl::osm::Area& a)
  : _prefix(prefix),  _value(std::to_string(a.id())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osm2ttl::osm::Node& n)
  : _prefix(prefix),  _value(std::to_string(n.id())) {
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
  // Trim strings
  _value = s.substr(begin - s.begin(), std::distance(begin, end.base()));
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::IRI::prefix() const noexcept {
  return _prefix;
}

// ____________________________________________________________________________
std::string osm2ttl::ttl::IRI::value() const noexcept {
  return _value;
}
