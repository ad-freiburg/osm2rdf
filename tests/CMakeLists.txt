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
    target_link_libraries(${TESTNAME} osm2ttl_library)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER vendor)
    # register test for global build target
    add_dependencies(build_tests ${TESTNAME})
    add_dependencies(run_tests run_${TESTNAME})
    add_custom_target(run_${TESTNAME} COMMAND ${TESTNAME} DEPENDS ${TESTNAME} WORKING_DIRECTORY "${PROJECT_DIR}")
endmacro()

add_custom_target(build_tests)
add_custom_target(run_tests)
package_add_test(BaselineTest Baseline.cpp)
package_add_test(GEOMETRY_AreaTest geometry/Area.cpp)
package_add_test(GEOMETRY_BoxTest geometry/Box.cpp)
package_add_test(GEOMETRY_LinestringTest geometry/Linestring.cpp)
package_add_test(GEOMETRY_LocationTest geometry/Location.cpp)
package_add_test(GEOMETRY_MultiPolygonTest geometry/MultiPolygon.cpp)
package_add_test(GEOMETRY_NodeTest geometry/Node.cpp)
package_add_test(GEOMETRY_PolygonTest geometry/Polygon.cpp)
package_add_test(GEOMETRY_RingTest geometry/Ring.cpp)
package_add_test(GEOMETRY_WayTest geometry/Way.cpp)
package_add_test(OSM_AreaTest osm/Area.cpp)
package_add_test(OSM_GeometryHandlerTest osm/GeometryHandler.cpp)
package_add_test(OSM_NodeTest osm/Node.cpp)
package_add_test(OSM_RelationTest osm/Relation.cpp)
package_add_test(OSM_WayTest osm/Way.cpp)
package_add_test(TTL_WriterTest ttl/Writer.cpp)
package_add_test(TTL_WriterGrammarTest ttl/Writer-Grammar.cpp)
package_add_test(UTIL_CacheFile util/CacheFile.cpp)
package_add_test(UTIL_DirectedGraphTest util/DirectedGraph.cpp)
package_add_test(UTIL_DirectedAcyclicGraphTest util/DirectedAcyclicGraph.cpp)
package_add_test(UTIL_OutputTest util/Output.cpp)
package_add_test(UTIL_ProgressBarTest util/ProgressBar.cpp)
