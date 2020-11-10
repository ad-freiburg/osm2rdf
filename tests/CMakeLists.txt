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
package_add_test(DirectedGraphTest DirectedGraph.cpp)
package_add_test(WriterTest Writer.cpp)
