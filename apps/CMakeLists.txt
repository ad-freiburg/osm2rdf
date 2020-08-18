add_executable(osm2ttl osm2ttl.cpp)
target_compile_features(osm2ttl PRIVATE cxx_std_17)

target_link_libraries(osm2ttl PRIVATE osm2ttl_library)
