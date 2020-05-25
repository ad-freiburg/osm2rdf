#----------------------------------------------------------------------
#
#  FindBoostconfig.cmake
#
#  Find the boost::config headers.
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
#      find_package(BoostConfig [version] [REQUIRED])
#      include_directories(SYSTEM ${BOOST_config_INCLUDE_DIR})
#
#    The version number is optional. If it is not set, any version of
#    protozero will do.
#
#      if(NOT BOOST_config_FOUND)
#          message(WARNING "boost::config not found!\n")
#      endif()
#
#----------------------------------------------------------------------
#
#  Variables:
#
#    BOOSTCONFIG_FOUND        - True if boost::config was found.
#    BOOSTCONFIG_INCLUDE_DIR  - Where to find include files.
#
#----------------------------------------------------------------------

# find include path
find_path(BOOSTTHROWEXCEPTION_INCLUDE_DIR boost/throw_exception.hpp
    PATH_SUFFIXES include
    PATHS ${CMAKE_SOURCE_DIR}/src/vendor/boost/throw_exception
)

set(BOOSTTHROWEXCEPTION_INCLUDE_DIRS "${BOOSTTHROWEXCEPTION_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BoostThrowException
    REQUIRED_VARS BOOSTTHROWEXCEPTION_INCLUDE_DIR
    VERSION_VAR _version)


#----------------------------------------------------------------------
