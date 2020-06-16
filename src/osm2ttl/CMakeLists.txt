file(GLOB_RECURSE osm2ttl_SRC *.cpp)

set(osm2ttl_main OSM2TTLMain.cpp)

list(REMOVE_ITEM osm2ttl_SRC ${osm2ttl_main})

include_directories(
	${OSM2TTL_INCLUDE_DIR}
)

configure_file (
  "_config.h.in"
  "_config.h"
)

add_executable(osm2ttl ${osm2ttl_main})
add_library(osm2ttl_dep ${osm2ttl_SRC})
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_dep)
target_link_libraries(${PROJECT_NAME} ${EXPAT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${BZIP2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
