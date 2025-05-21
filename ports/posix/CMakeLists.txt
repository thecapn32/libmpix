# SPDX-License-Identifier: Apache-2.0

file(REAL_PATH ../.. LIBMPIX_DIR)

set(LIBMPIX_SRC
  ${LIBMPIX_DIR}/src/formats.c
  ${LIBMPIX_DIR}/src/image.c
  ${LIBMPIX_DIR}/src/op_bayer.c
  ${LIBMPIX_DIR}/src/op_convert.c
  ${LIBMPIX_DIR}/src/op_kernel.c
  ${LIBMPIX_DIR}/src/op_palettize.c
  ${LIBMPIX_DIR}/src/op_resize.c
  ${LIBMPIX_DIR}/src/print.c
  ${LIBMPIX_DIR}/src/stats.c

  ${LIBMPIX_DIR}/ports/posix/port.c
)

set(LIBMPIX_GENLIST_SRC
  ${LIBMPIX_SRC}
)

add_library(libmpix INTERFACE
  ${LIBMPIX_SRC}
)

target_sources(libmpix INTERFACE
  ${LIBMPIX_SRC}
)

message("LIBMPIX_SRC=${LIBMPIX_SRC}")

target_include_directories(libmpix INTERFACE
  ${LIBMPIX_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_custom_target(libmpix_genlist
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/mpix
  COMMAND python ${LIBMPIX_DIR}/scripts/genlist.py ${LIBMPIX_GENLIST_SRC}
    >${CMAKE_CURRENT_BINARY_DIR}/include/mpix/genlist.h
  DEPENDS ${LIBMPIX_GENLIST_SRC}
)

add_dependencies(libmpix libmpix_genlist)
