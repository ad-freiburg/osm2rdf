# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../include/osm2ttl/*.h")
file(GLOB_RECURSE CPP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(osm2ttl_library ${CPP_LIST} ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(osm2ttl_library PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(osm2ttl_library PRIVATE
  Boost::boost
  Threads::Threads
  ${EXPAT_LIBRARIES}
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES})

# Link OpenMP if found
if(OpenMP_CXX_FOUND)
    target_link_libraries(osm2ttl_library PUBLIC OpenMP::OpenMP_CXX)
endif()

# All users of this library will need at least C++17
target_compile_features(osm2ttl_library PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST})
