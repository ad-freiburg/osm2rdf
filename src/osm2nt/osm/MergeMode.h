
// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

#ifndef OSM2NT_OSM_MERGEMODE_H_
#define OSM2NT_OSM_MERGEMODE_H_

namespace osm2nt {
namespace osm {

enum class MergeMode {
  DELETE_FIRST,
  DELETE_SECOND,
  MERGE
};

}  // namespace osm
}  // namespace osm2nt

#endif  // OSM2NT_OSM_MERGEMODE_H_
