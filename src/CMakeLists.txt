set(OSM2NT_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if (COMMAND cpplint_add_subdirectory)
	cpplint_add_subdirectory(osm2nt)
endif()

include_directories(
	${OSM2NT_INCLUDE_DIR}
)

include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})
include_directories(SYSTEM ${PROTOZERO_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTVARIANT_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTTYPEINDEX_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTCONFIG_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTCONTAINERHASH_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTSTATICASSERT_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTTHROWEXCEPTION_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTASSERT_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTCORE_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTTYPETRAITS_INCLUDE_DIRS})
include_directories(SYSTEM ${BOOSTMPL_INCLUDE_DIRS})
include_directories(SYSTEM ${EXPAT_INCLUDE_DIRS})
include_directories(SYSTEM ${BZIP2_INCLUDE_DIR})
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})

add_subdirectory(osm2nt)
