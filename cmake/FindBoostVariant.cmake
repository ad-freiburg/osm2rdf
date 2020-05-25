#----------------------------------------------------------------------
#
#  FindBoostVariant.cmake
#
#  Find the boost::variant headers.
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
#      find_package(Boost_Variant [version] [REQUIRED])
#      include_directories(SYSTEM ${BOOST_VARIANT_INCLUDE_DIR})
#
#    The version number is optional. If it is not set, any version of
#    protozero will do.
#
#      if(NOT BOOST_VARIANT_FOUND)
#          message(WARNING "boost::variant not found!\n")
#      endif()
#
#----------------------------------------------------------------------
#
#  Variables:
#
#    BOOSTVARIANT_FOUND        - True if boost::variant was found.
#    BOOSTVARIANT_INCLUDE_DIR  - Where to find include files.
#
#----------------------------------------------------------------------

# find include path
find_path(BOOSTVARIANT_INCLUDE_DIR boost/variant.hpp
    PATH_SUFFIXES include
    PATHS ${CMAKE_SOURCE_DIR}/src/vendor/boost/variant
)

set(BOOSTVARIANT_INCLUDE_DIRS "${BOOSTVARIANT_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BoostVariant
    REQUIRED_VARS BOOSTVARIANT_INCLUDE_DIR
    VERSION_VAR _version)


#----------------------------------------------------------------------
