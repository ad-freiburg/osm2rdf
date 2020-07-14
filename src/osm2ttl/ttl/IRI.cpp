// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/ttl/IRI.h"

#include <cctype>
#include <iterator>
#include <string>
#include <iostream>

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/RelationMember.h"
#include "osm2ttl/osm/Way.h"

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osm2ttl::osm::Area& a)
  : _prefix(prefix),  _value(std::to_string(a.id())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osm2ttl::osm::Node& n)
  : _prefix(prefix),  _value(std::to_string(n.id())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix,
                       const osm2ttl::osm::Relation& r)
  : _prefix(prefix),  _value(std::to_string(r.id())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix,
                       const osm2ttl::osm::RelationMember& m)
  : _prefix(prefix),  _value(std::to_string(m.id())) {
}

// ____________________________________________________________________________
osm2ttl::ttl::IRI::IRI(const std::string& prefix, const osm2ttl::osm::Way& w)
  : _prefix(prefix),  _value(std::to_string(w.id())) {
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
