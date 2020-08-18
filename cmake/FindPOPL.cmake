#----------------------------------------------------------------------
#
#  FindPOPL.cmake
#
#  Find the popl headers.
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
#      find_package(POPL [version] [REQUIRED])
#      include_directories(SYSTEM ${POPL_INCLUDE_DIR})
#
#    The version number is optional. If it is not set, any version of
#    protozero will do.
#
#      if(NOT POPL_FOUND)
#          message(WARNING "popl not found!\n")
#      endif()
#
#----------------------------------------------------------------------
#
#  Variables:
#
#    POPL_FOUND        - True if boost::config was found.
#    POPL_INCLUDE_DIR  - Where to find include files.
#
#----------------------------------------------------------------------

# find include path
find_path(POPL_INCLUDE_DIR popl.hpp
    PATH_SUFFIXES include
    PATHS ${CMAKE_SOURCE_DIR}/vendor/baidaix/popl
)

set(POPL_INCLUDE_DIRS "${POPL_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(POPL
    REQUIRED_VARS POPL_INCLUDE_DIR
    VERSION_VAR _version)


#----------------------------------------------------------------------
