# Copyright 2020, University of Freiburg
# Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

# This file is part of osm2rdf.
#
# osm2rdf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# osm2rdf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with osm2rdf.  If not, see <https://www.gnu.org/licenses/>.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../include/osm2rdf/*.h")
file(GLOB_RECURSE CPP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(osm2rdf_library ${CPP_LIST} ${HEADER_LIST})

# Add gtest header for FRIEND_TEST
target_include_directories(osm2rdf_library PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../vendor/google/googletest/googletest/include")

# We need this directory, and users of our library will need it too
target_include_directories(osm2rdf_library PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../include")

target_link_libraries(osm2rdf_library PRIVATE
	    spatialjoin-dev
		pb_util
        Threads::Threads
        ${EXPAT_LIBRARIES}
        ${BZIP2_LIBRARIES}
        ${ZLIB_LIBRARIES})

# Link OpenMP if found
if (OpenMP_CXX_FOUND)
    target_link_libraries(osm2rdf_library PUBLIC OpenMP::OpenMP_CXX)
endif ()

# All users of this library will need at least C++17
target_compile_features(osm2rdf_library PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST})
