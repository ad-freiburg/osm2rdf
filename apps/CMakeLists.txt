add_executable(osm2ttl osm2ttl.cpp)
target_link_libraries(osm2ttl PRIVATE osm2ttl_library)

add_executable(osm2ttl-stats osm2ttl-stats.cpp)
target_link_libraries(osm2ttl-stats PRIVATE osm2ttl_library)