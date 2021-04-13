# Copyright 2020, University of Freiburg
# Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

# This file is part of osm2ttl.
#
# osm2ttl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# osm2ttl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

add_executable(osm2ttl osm2ttl.cpp)
target_link_libraries(osm2ttl PRIVATE osm2ttl_library)

add_executable(osm2ttl-stats osm2ttl-stats.cpp)
target_link_libraries(osm2ttl-stats PRIVATE osm2ttl_library)