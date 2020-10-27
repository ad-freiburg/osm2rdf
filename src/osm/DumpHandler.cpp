// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#include "osm2ttl/osm/DumpHandler.h"

#include "osmium/osm/area.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/relation.hpp"
#include "osmium/osm/way.hpp"

#include "osm2ttl/config/Config.h"

#include "osm2ttl/ttl/Writer.h"

#include "osm2ttl/osm/Area.h"
#include "osm2ttl/osm/Node.h"
#include "osm2ttl/osm/Relation.h"
#include "osm2ttl/osm/Way.h"

// ____________________________________________________________________________
template<typename W>
osm2ttl::osm::DumpHandler<W>::DumpHandler(const osm2ttl::config::Config& config,
  osm2ttl::ttl::Writer<W>* writer) : _config(config),
  _writer(writer) {
}

// ____________________________________________________________________________
template<typename W>
template<typename T>
void osm2ttl::osm::DumpHandler<W>::write(const T& o) {
  _writer->write(o);
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::area(const osm2ttl::osm::Area& area) {
  write(area);
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::node(const osm2ttl::osm::Node& node) {
  write(node);
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::relation(const osm2ttl::osm::Relation& relation) {
  write(relation);
}

// ____________________________________________________________________________
template<typename W>
void osm2ttl::osm::DumpHandler<W>::way(const osm2ttl::osm::Way& way) {
  write(way);
}

// ____________________________________________________________________________
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::NT>;
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::TTL>;
template class osm2ttl::osm::DumpHandler<osm2ttl::ttl::format::QLEVER>;
