# SPDX-License-Identifier: Apache-2.0

# Example CMake project for using libmpix on a POSIX system such as Linux, BSD, Mac OSX...

cmake_minimum_required(VERSION 3.20)

project(app LANGUAGES C)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall -Wextra -Wno-unused-parameter)
add_executable(mpix-simple-isp-demo main.c)
add_subdirectory(../../ports/posix libmpix)

set(CMSISCORE ../../lib/CMSIS/CMSIS CACHE PATH "")
add_subdirectory(../../lib/CMSIS-DSP cmsis_dsp)
target_include_directories(CMSISDSP PUBLIC ${CMSISCORE}/Core/Include)

target_link_libraries(mpix-simple-isp-demo PRIVATE CMSISDSP m)
target_link_libraries(mpix-simple-isp-demo PUBLIC libmpix)
libmpix_init(mpix-simple-isp-demo)
