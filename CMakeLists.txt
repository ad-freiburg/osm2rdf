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

cmake_minimum_required(VERSION 3.16...3.18)
# ----------------------------------------------------------------------------
# Project
# ----------------------------------------------------------------------------
project(
        osm2rdf
        VERSION 0.2
        DESCRIPTION "Convert OSM data to RDF"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT error)

# export compile commands to tools like clang
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------------
# Global settings
# ----------------------------------------------------------------------------

# set(CMAKE_VERBOSE_MAKEFILE ON)

option(ENABLE_GEOMETRY_STATISTIC "Write geometry statistics if enabled" 0)

# Force static libraries
set(BUILD_SHARED_LIBS OFF)

# Enable verbose makefile
if (ENABLE_GEOMETRY_STATISTIC)
	# Enable geometry statistics
	add_definitions(-DENABLE_GEOMETRY_STATISTIC)
endif()

add_compile_options(-Wall -Wextra -Wno-missing-field-initializers -Wno-stringop-overread)
add_compile_options(-DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0)

add_compile_options(-march=native)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")

# ----------------------------------------------------------------------------
# Configure dependencies
# ----------------------------------------------------------------------------
find_package(EXPAT REQUIRED)
find_package(BZip2 REQUIRED)
find_package(OpenMP)

# zlib-ng
set(ZLIB_COMPAT ON)
set(WITH_GTEST OFF)
set(ZLIB_ENABLE_TESTS OFF)
add_subdirectory(vendor/zlib-ng/zlib-ng)

set(ZLIB_USE_STATIC_LIBS ON)
set(ZLIB_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vendor/zlib-ng/zlib-ng")
set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/vendor/zlib-ng/zlib-ng/libz.a")

# Disable installation of google stuff
set(INSTALL_GMOCK OFF)
set(INSTALL_GTEST OFF)

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
set(OSMIUM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vendor/osmcode/libosmium/include")
find_package(Osmium REQUIRED COMPONENTS pbf xml)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})

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
endif ()

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

# Configure project
configure_file(
        "${PROJECT_SOURCE_DIR}/include/osm2rdf/Version.h.in"
        "${PROJECT_SOURCE_DIR}/include/osm2rdf/Version.h"
)

# The compiled library code is here
add_subdirectory(src)

# libspatialjoin
add_subdirectory(vendor/spatialjoin)

# The executable code is here
add_subdirectory(apps)

# Install target settings
install(
        FILES build/apps/osm2rdf DESTINATION bin
        PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE COMPONENT binaries
)
