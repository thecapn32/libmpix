# SPDX-License-Identifier: Apache-2.0

# CMake rules to build libmpix in an firmware image, for any board supporteded by Zephyr.

zephyr_library()

# Zephyr-specific implementations

function(libmpix_add_include_directory target directory)
  zephyr_include_directories(${directory})
endfunction()

# Common implementations

file(REAL_PATH .. LIBMPIX_DIR)
include(${LIBMPIX_DIR}/ports/common.cmake)

if(Zephyr_VERSION VERSION_EQUAL "3.7")
  zephyr_library_sources(${LIBMPIX_SRC} ${LIBMPIX_DIR}/ports/zephyr_3_7/port.c)
endif()

if(Zephyr_VERSION VERSION_EQUAL "4.0")
  zephyr_library_sources(${LIBMPIX_SRC} ${LIBMPIX_DIR}/ports/zephyr_4_0/port.c)
endif()

if(Zephyr_VERSION VERSION_EQUAL "4.1.0")
  zephyr_library_sources(${LIBMPIX_SRC} ${LIBMPIX_DIR}/ports/zephyr_4_1/port.c)
endif()

if(Zephyr_VERSION VERSION_EQUAL "4.1.99")
  zephyr_library_sources(${LIBMPIX_SRC} ${LIBMPIX_DIR}/ports/zephyr_main/port.c)
endif()
