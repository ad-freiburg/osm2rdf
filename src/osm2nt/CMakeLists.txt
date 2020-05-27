file(GLOB_RECURSE osm2nt_SRC *.cpp)

set(osm2nt_main OSM2NTMain.cpp)

list(REMOVE_ITEM osm2nt_SRC ${osm2nt_main})

include_directories(
	${OSM2NT_INCLUDE_DIR}
)

configure_file (
  "_config.h.in"
  "_config.h"
)

add_executable(osm2nt ${osm2nt_main})
add_library(osm2nt_dep ${osm2nt_SRC})
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_dep)
target_link_libraries(${PROJECT_NAME} ${EXPAT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${BZIP2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
