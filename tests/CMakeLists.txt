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

set(INSTALL_GTEST OFF CACHE BOOL "Suppressing installation of googletest" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/google/googletest" "vendor/google/googletest")

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER vendor)
set_target_properties(gtest_main PROPERTIES FOLDER vendor)
set_target_properties(gmock PROPERTIES FOLDER vendor)
set_target_properties(gmock_main PROPERTIES FOLDER vendor)

macro(package_add_test TESTNAME)
    # create an executable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main fuction to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    target_link_libraries(${TESTNAME} osm2rdf_library)
    target_link_libraries(${TESTNAME} pb_util)
    target_link_libraries(${TESTNAME} spatialjoin-dev)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER vendor)
    # register test for global build target
    add_dependencies(build_tests ${TESTNAME})
    add_dependencies(run_tests run_${TESTNAME})
	add_custom_target(run_${TESTNAME} COMMAND ${TESTNAME} DEPENDS ${TESTNAME} WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endmacro()

add_custom_target(build_tests)
add_custom_target(run_tests)
package_add_test(BaselineTest Baseline.cpp)
package_add_test(E2ETest E2E.cpp)
package_add_test(CONFIG_ConfigTest config/Config.cpp)
package_add_test(ISSUES_15Test issues/Issue15.cpp)
package_add_test(ISSUES_24Test issues/Issue24.cpp)
package_add_test(ISSUES_28Test issues/Issue28.cpp)
package_add_test(OSM_AreaTest osm/Area.cpp)
package_add_test(OSM_FactHandlerTest osm/FactHandler.cpp)
package_add_test(OSM_NodeTest osm/Node.cpp)
package_add_test(OSM_OsmiumHandlerTest osm/OsmiumHandler.cpp)
package_add_test(OSM_RelationTest osm/Relation.cpp)
package_add_test(OSM_RelationMemberTest osm/RelationMember.cpp)
package_add_test(OSM_TagListTest osm/TagList.cpp)
package_add_test(OSM_WayTest osm/Way.cpp)
package_add_test(TTL_WriterTest ttl/Writer.cpp)
package_add_test(TTL_WriterGrammarTest ttl/Writer-Grammar.cpp)
package_add_test(UTIL_CacheFile util/CacheFile.cpp)
package_add_test(UTIL_DirectedGraphTest util/DirectedGraph.cpp)
package_add_test(UTIL_DirectedAcyclicGraphTest util/DirectedAcyclicGraph.cpp)
package_add_test(UTIL_OutputTest util/Output.cpp)
package_add_test(UTIL_ProgressBarTest util/ProgressBar.cpp)
package_add_test(UTIL_TimeTest util/Time.cpp)

# copy test files to binary directory to make sure they can be found
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

