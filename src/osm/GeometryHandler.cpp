// Copyright 2020 - 2022, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>
//          Patrick Brosi <brosi@cs.uni-freiburg.de>
//          Hannah Bast <bast@cs.uni-freiburg.de>

// This file is part of osm2rdf.
//
// osm2rdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2rdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "osm2rdf/config/Config.h"
#include "osm2rdf/osm/Area.h"
#include "osm2rdf/osm/Constants.h"
#include "osm2rdf/osm/FactHandler.h"
#include "osm2rdf/osm/GeometryHandler.h"
#include "osm2rdf/ttl/Constants.h"
#include "osm2rdf/ttl/Writer.h"
#include "osm2rdf/util/Time.h"

using osm2rdf::osm::Area;
using osm2rdf::osm::GeometryHandler;
using osm2rdf::osm::Node;
using osm2rdf::osm::Relation;
using osm2rdf::osm::Way;

// ____________________________________________________________________________
template <typename W>
GeometryHandler<W>::GeometryHandler(const osm2rdf::config::Config& config,
                                    osm2rdf::ttl::Writer<W>* writer)
    : _config(config),
      _writer(writer) {
}

// ___________________________________________________________________________
template <typename W>
GeometryHandler<W>::~GeometryHandler() = default;

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::relation(const Relation& rel) {
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::area(const Area& area) {
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::node(const Node& node) {
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::way(const Way& way) {
}

// ____________________________________________________________________________
template <typename W>
void GeometryHandler<W>::calculateRelations() {
}


// ____________________________________________________________________________
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::NT>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::TTL>;
template class osm2rdf::osm::GeometryHandler<osm2rdf::ttl::format::QLEVER>;
