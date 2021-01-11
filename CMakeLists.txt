cmake_minimum_required(VERSION 3.16...3.18)

project(
        osm2ttl
        VERSION 0.5
        DESCRIPTION "Convert osm to ttl"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)
find_package(EXPAT REQUIRED)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenMP REQUIRED)

find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if (GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif ()
    endif ()
endif ()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/google/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif ()

find_package(POPL REQUIRED)
include_directories(SYSTEM ${POPL_INCLUDE_DIR})
find_package(Protozero REQUIRED)
include_directories(SYSTEM ${PROTOZERO_INCLUDE_DIR})
find_package(Osmium REQUIRED COMPONENTS pbf xml)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})
add_subdirectory(vendor/bingmann/stxxl)
include_directories(${STXXL_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    set(default_build_type "Release")
    if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
        set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
        # Set the possible values of build type for cmake-gui
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif ()

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    option(PACKAGE_TESTS "Build the tests" ON)
    if (PACKAGE_TESTS)
        enable_testing()
        include(GoogleTest)
        add_subdirectory(tests)
    endif ()
    option(PACKAGE_BENCHMARKS "Build the benchmarks" ON)
    if (PACKAGE_BENCHMARKS)
        enable_testing()
        add_subdirectory(benchmarks)
    endif ()
endif ()

find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
)

# Based on https://github.com/ad-freiburg/pfaedle/blob/master/cmake/GetGitRevisionDescription.cmake
function(get_git_sha1 _var)
    if (NOT GIT_FOUND)
        find_package(Git QUIET)
    endif ()

    execute_process(COMMAND
            "${GIT_EXECUTABLE}"
            rev-parse --short HEAD --
            WORKING_DIRECTORY
            "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE
            res
            OUTPUT_VARIABLE
            out
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT res EQUAL 0)
        set(out "${out}-${res}-NOTFOUND")
    endif ()
    set(${_var} "${out}" PARENT_SCOPE)
endfunction()
function(get_git_is_dirty _var)
    if (NOT GIT_FOUND)
        find_package(Git QUIET)
    endif ()

    execute_process(COMMAND
            "${GIT_EXECUTABLE}"
            diff-index --name-only HEAD --
            WORKING_DIRECTORY
            "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE
            res
            OUTPUT_VARIABLE
            out
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT res EQUAL 0)
        set(out "${out}-${res}-NOTFOUND")
    endif ()

    if (NOT "${out}" STREQUAL "")
        set(IS_DIRTY "dirty")
    else ()
        set(IS_DIRTY "")
    endif ()
    set(${_var} "${IS_DIRTY}" PARENT_SCOPE)
endfunction()

get_git_sha1(VERSION_GIT)
get_git_is_dirty(VERSION_GIT_IS_DIRTY)
if ("${VERSION_GIT_IS_DIRTY}" STREQUAL "")
    set(VERSION_GIT_FULL "${VERSION_GIT}")
else ()
    set(VERSION_GIT_FULL "${VERSION_GIT}-${VERSION_GIT_IS_DIRTY}")
endif ()


configure_file(
        "${PROJECT_SOURCE_DIR}/include/osm2ttl/Version.h.in"
        "${PROJECT_SOURCE_DIR}/include/osm2ttl/Version.h"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STXXL_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fstack-check")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0 -march=native")
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -Og -DLOGLEVEL=3 -DOSM2NT_DBG=1")
#message(STATUS "CMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}")
#set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} -DLOGLEVEL=2")
#message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL=${CMAKE_CXX_FLAGS_MINSIZEREL}")
##set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -DLOGLEVEL=2")
#message(STATUS "CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DLOGLEVEL=3 -g")
#message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
if (CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=-*,readability-*")
endif ()

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)
