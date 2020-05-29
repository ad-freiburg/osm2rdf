cmake_minimum_required (VERSION 3.6)

project (osm2nt CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


if (CMAKE_BUILD_TYPE)
	string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	set(CPPLINT "${CMAKE_SOURCE_DIR}/cpplint.py")
	include(cmake/cpplint.cmake)
endif()

set(CPPLINT_PROJECT_ROOT "src")

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build")


find_package(Osmium)
if (NOT OSMIUM_FOUND)
	message(WARNING "No global libosmium found, using local copy")
endif()

find_package(Protozero)
if (NOT PROTOZERO_FOUND)
	message(WARNING "No global protozero found, using local copy")
endif()

find_package(BoostVariant)
if (NOT BOOSTVARIANT_FOUND)
	message(WARNING "No global boost::variant found, using local copy")
endif()

find_package(BoostTypeIndex)
if (NOT BOOSTTYPEINDEX_FOUND)
	message(WARNING "No global boost::type_index found, using local copy")
endif()

find_package(BoostConfig)
if (NOT BOOSTCONFIG_FOUND)
	message(WARNING "No global boost::config found, using local copy")
endif()

find_package(BoostContainerHash)
if (NOT BOOSTCONTAINERHASH_FOUND)
	message(WARNING "No global boost::container_hash found, using local copy")
endif()

find_package(BoostStaticAssert)
if (NOT BOOSTSTATICASSERT_FOUND)
	message(WARNING "No global boost::static_assert found, using local copy")
endif()

find_package(BoostThrowException)
if (NOT BOOSTTHROWEXCEPTION_FOUND)
	message(WARNING "No global boost::throw_exception found, using local copy")
endif()

find_package(BoostAssert)
if (NOT BOOSTASSERT_FOUND)
	message(WARNING "No global boost::assert found, using local copy")
endif()

find_package(BoostCore)
if (NOT BOOSTCORE_FOUND)
	message(WARNING "No global boost::core found, using local copy")
endif()

find_package(BoostTypeTraits)
if (NOT BOOSTTYPETRAITS_FOUND)
	message(WARNING "No global boost::type_traits found, using local copy")
endif()

find_package(BoostMPL)
if (NOT BOOSTMPL_FOUND)
	message(WARNING "No global boost::mpl found, using local copy")
endif()

find_package(EXPAT REQUIRED)
if (NOT EXPAT_FOUND)
	message(WARNING "No global expat found")
endif()

find_package(BZip2 REQUIRED)
if (NOT BZIP2_FOUND)
	message(WARNING "No global bzip2 found")
endif()

find_package(ZLIB REQUIRED)
if (NOT ZLIB_FOUND)
	message(WARNING "No global zlib found")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Wextra -Wno-missing-field-initializers -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0")
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS} -Og -g -DLOGLEVEL=3 -DOSM2NT_DBG=1")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS} -DLOGLEVEL=2 -Os")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS} -DLOGLEVEL=2 -O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -g -DLOGLEVEL=3")
set(CMAKE_CXX_CLANG_TIDY           "clang-tidy;-checks=-*,readability-*")

# export compile commands to tools like clang
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	execute_process(
		COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	if ((GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8))
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else ()
		message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8 or greater!")
	endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else ()
	message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# http://brianmilco.blogspot.de/2012/11/cmake-automatically-use-git-tags-as.html
include(GetGitRevisionDescription)
git_get_tag(VERSION_GIT)
get_git_is_dirty(VERSION_GIT_IS_DIRTY)
if ("${VERSION_GIT_IS_DIRTY}" STREQUAL "")
	set(VERSION_GIT_FULL "${VERSION_GIT}")
else()
	set(VERSION_GIT_FULL "${VERSION_GIT}-${VERSION_GIT_IS_DIRTY}")
endif()

# Download submodules if needed

if(NOT OSMIUM_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/libosmium/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/libosmium
    )
endif()
if(NOT PROTOZERO_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/protozero/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/protozero
    )
endif()
if(NOT BOOSTVARIANT_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/variant/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/variant
    )
	find_package(BoostVariant)
endif()
if(NOT BOOSTTYPEINDEX_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/type_index/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/type_index
    )
endif()
if(NOT BOOSTCONFIG_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/config/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/config
    )
endif()
if(NOT BOOSTCONTAINERHASH_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/container_hash/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/container_hash
    )
endif()
if(NOT BOOSTSTATICASSERT_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/static_assert/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/static_assert
    )
endif()
if(NOT BOOSTTHROWEXCEPTION_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/throw_exception/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/throw_exception
    )
endif()
if(NOT BOOSTASSERT_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/assert/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/assert
    )
endif()
if(NOT BOOSTCORE_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/core/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/core
    )
endif()
if(NOT BOOSTTYPETRAITS_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/type_traits/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/type_traits
    )
endif()
if(NOT BOOSTMPL_FOUND AND NOT EXISTS ${CMAKE_SOURCE_DIR}/src/vendor/boost/mpl/.git)
    execute_process(
		COMMAND git submodule update --init --recursive ${CMAKE_SOURCE_DIR}/src/vendor/boost/mpl
    )
endif()

add_subdirectory(src)

# tests

add_test("utilTest" utilTest)

# custom eval target

add_custom_target(
	eval
	COMMAND make
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}//eval
)

# handles install target
install(
	FILES osm2nt.cfg DESTINATION etc/${PROJECT_NAME} COMPONENT config PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)

install(
	FILES build/osm2nt DESTINATION bin
	PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE COMPONENT binaries
)
