prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
libdir=${LIB_DESTINATION}
includedir=${CMAKE_INSTALL_PREFIX}/include

Name: libstreamanalyzer
Description: C++ library for extracting text and metadata from files and streams
Requires: libstreams libpthread
Version: ${STRIGI_VERSION_MAJOR}.${STRIGI_VERSION_MINOR}.${STRIGI_VERSION_PATCH}
Libs: -L${libdir} -lstreamanalyzer
Cflags: -I${includedir} -I${libdir}/include
