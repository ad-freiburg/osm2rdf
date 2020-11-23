add_executable(osm2ttl osm2ttl.cpp)
target_link_libraries(osm2ttl PRIVATE osm2ttl_library)

add_executable(osm2ttl-maxid osm2ttl-maxid.cpp)
target_link_libraries(osm2ttl-maxid PRIVATE osm2ttl_library)