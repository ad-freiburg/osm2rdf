#----------------------------------------------------------------------
#
#  FindBoosttype_index.cmake
#
#  Find the boost::type_index headers.
#
#----------------------------------------------------------------------
#
#  Usage:
#
#    Copy this file somewhere into your project directory, where cmake can
#    find it. Usually this will be a directory called "cmake" which you can
#    add to the CMake module search path with the following line in your
#    CMakeLists.txt:
#
#      list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
#
#    Then add the following in your CMakeLists.txt:
#
#      find_package(BoostTypeIndex [version] [REQUIRED])
#      include_directories(SYSTEM ${BOOST_type_index_INCLUDE_DIR})
#
#    The version number is optional. If it is not set, any version of
#    protozero will do.
#
#      if(NOT BOOST_type_index_FOUND)
#          message(WARNING "boost::type_index not found!\n")
#      endif()
#
#----------------------------------------------------------------------
#
#  Variables:
#
#    BOOSTTYPEINDEX_FOUND        - True if boost::type_index was found.
#    BOOSTTYPEINDEX_INCLUDE_DIR  - Where to find include files.
#
#----------------------------------------------------------------------

# find include path
find_path(BOOSTTYPEINDEX_INCLUDE_DIR boost/type_index.hpp
    PATH_SUFFIXES include
    PATHS ${CMAKE_SOURCE_DIR}/src/vendor/boost/type_index
)

set(BOOSTTYPEINDEX_INCLUDE_DIRS "${BOOSTTYPEINDEX_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BoostTypeIndex
    REQUIRED_VARS BOOSTTYPEINDEX_INCLUDE_DIR
    VERSION_VAR _version)


#----------------------------------------------------------------------
