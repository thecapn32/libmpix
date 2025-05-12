# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_include_directories(../include)
add_subdirectory_ifdef(CONFIG_MPIX ../src)

zephyr_linker_sources(DATA_SECTIONS mpix.ld)
