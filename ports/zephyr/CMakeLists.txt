# SPDX-License-Identifier: Apache-2.0

# CMake rules to build libmpix in an firmware image, for any board supporteded by Zephyr.

zephyr_library()

function(libmpix_add_include_directory target directory)
  zephyr_include_directories(${directory})
endfunction()

file(REAL_PATH ../.. LIBMPIX_DIR)
include(${LIBMPIX_DIR}/ports/common.cmake)

zephyr_library_sources(${LIBMPIX_SRC} ${LIBMPIX_DIR}/ports/zephyr/port.c)
