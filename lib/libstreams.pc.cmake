prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
libdir=${LIB_DESTINATION}
includedir=${INCLUDE_DESTINATION}

Name: libstreams
Description: C++ streams for reading data as streams from various file formats
Version: ${LIBSTREAMS_VERSION}

Libs: -L${LIB_DESTINATION} -lstreams
Cflags: -I${INCLUDE_DESTINATION}
