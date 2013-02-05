project (libstreams)

##### cmake settings #####
cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MacroCheckGccVisibility)
enable_testing()

# Used to report found packages
include(FeatureSummary)

# Used to create config files
include(CMakePackageConfigHelpers)

# Used to set some generic paths
include(GNUInstallDirs)

##### global variables #####

# libstreams defines the definitions shared by the Strigi projects
set(STRIGI_VERSION_MAJOR 0 CACHE INT "Major Strigi version number" FORCE)
set(STRIGI_VERSION_MINOR 7 CACHE INT "Minor Strigi version number" FORCE)
set(STRIGI_VERSION_PATCH 8 CACHE INT "Release Strigi version number" FORCE)
set(STRIGI_VERSION_STRING "${STRIGI_VERSION_MAJOR}.${STRIGI_VERSION_MINOR}.${STRIGI_VERSION_PATCH}" CACHE STRING "Strigi version string" FORCE)
set(LIBSTREAMS_VERSION ${STRIGI_VERSION_STRING})

# (Absolute) paths definitions
set(LIB_DESTINATION     "${CMAKE_INSTALL_FULL_LIBDIR}")
set(INCLUDE_DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}")

# Set up RPATH handling, so the libs are found if they are installed to a non-standard location.
# By default cmake builds the targets with full RPATH to everything in the build directory,
# but then removes the RPATH when installing.
# These two options below make it set the RPATH of the installed targets to all
# RPATH directories outside the current CMAKE_BINARY_DIR and also the library
# install directory. Alex
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE)
set(CMAKE_INSTALL_RPATH ${LIB_DESTINATION})

if(NOT MSVC AND NOT MINGW AND NOT CMAKE_SYSTEM MATCHES "SunOS-5*.")
  add_definitions(-fPIC)
endif()

##### environment inspection #####

# check for visibility support
macro_check_gcc_visibility(__STRIGI_HAVE_GCC_VISIBILITY)

# Generate include/strigi/strigiconfig.h and lib/config.h
include(ConfigureChecks.cmake)

# check for required packages
find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
                       DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                      )

find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
                       DESCRIPTION "Support for BZip2 compressed files and data streams"
                       URL "http://www.bzip.org"
                       TYPE REQUIRED
                      )

find_package(Iconv)
set_package_properties(Iconv PROPERTIES
                       TYPE REQUIRED
                      )

find_package(Threads)
set_package_properties(Threads PROPERTIES
                       DESCRIPTION "The thread library of the system"
                       TYPE REQUIRED
                      )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

##### building and testing #####
# every directory needs the headers that will be installed
include_directories(${libstreams_SOURCE_DIR}/include
                    ${libstreams_BINARY_DIR}/include
                    ${libstreams_SOURCE_DIR}/lib
                    ${libstreams_BINARY_DIR}/lib
                    ${ZLIB_INCLUDE_DIR}
                    ${BZIP2_INCLUDE_DIR}
                    ${ICONV_INCLUDE_DIR}
)

add_subdirectory(lib)
add_subdirectory(tests)

# Create the CMake Config files
configure_package_config_file(StrigiConfig.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/StrigiConfig.cmake
                              INSTALL_DESTINATION ${LIB_DESTINATION}/cmake/Strigi
                              PATH_VARS INCLUDE_DESTINATION LIB_DESTINATION
)

configure_package_config_file(LibStreamsConfig.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/LibStreamsConfig.cmake
                              INSTALL_DESTINATION ${LIB_DESTINATION}/cmake/LibStreams
                              PATH_VARS INCLUDE_DESTINATION LIB_DESTINATION
)

write_basic_package_version_file(${libstreams_BINARY_DIR}/StrigiConfigVersion.cmake
                                 VERSION ${STRIGI_VERSION_STRING}
                                 COMPATIBILITY SameMajorVersion
                                )

write_basic_package_version_file(${libstreams_BINARY_DIR}/LibStreamsConfigVersion.cmake
                                 VERSION ${STRIGI_VERSION_STRING}
                                 COMPATIBILITY SameMajorVersion
                                )


##### installing #####

# all installed files are listed here
# headers
file(GLOB STRIGI_HEADERS include/strigi/*.h)
install(FILES
      ${STRIGI_HEADERS}
      ${strigi_config_output}
      ${strigi_extra_config_output}
        DESTINATION ${INCLUDE_DESTINATION}/strigi
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/StrigiConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/StrigiConfigVersion.cmake
        DESTINATION ${LIB_DESTINATION}/cmake/Strigi)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LibStreamsConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/LibStreamsConfigVersion.cmake
        DESTINATION ${LIB_DESTINATION}/cmake/LibStreams)

install(EXPORT StreamsExport FILE LibStreamsTargets.cmake
        DESTINATION ${LIB_DESTINATION}/cmake/LibStreams)

# library
if(NOT WIN32)
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/libstreams.pc
              DESTINATION ${LIB_DESTINATION}/pkgconfig)
endif()
