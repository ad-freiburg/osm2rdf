file(GLOB_RECURSE osm2nt_SRC *.cpp)

set(osm2nt_main OSM2NTMain.cpp)

list(REMOVE_ITEM osm2nt_SRC ${osm2nt_main})

include_directories(
	${OSM2NT_INCLUDE_DIR}
)

add_executable(osm2nt ${osm2nt_main})
add_library(osm2nt_dep ${osm2nt_SRC})
