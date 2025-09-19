# SPDX-License-Identifier: Apache-2.0
# CMake toolchain file for ARM Cortex-M55 cross-compilation

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Toolchain paths
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Specify the cross compiler
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)

# Utility tools
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)
set(CMAKE_DEBUGGER ${TOOLCHAIN_PREFIX}gdb)
set(CMAKE_CPPFILT ${TOOLCHAIN_PREFIX}c++filt)

# We are cross compiling so we don't want to run any programs
set(CMAKE_CROSSCOMPILING TRUE)

# Skip compiler tests
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Search for libraries and headers in the target directories only
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Cortex-M55 specific flags
set(CPU_FLAGS
    "-mcpu=cortex-m55"
    "-mfloat-abi=hard"
    "-mfpu=auto"
)

# Common flags for both C and CXX
set(COMMON_FLAGS
    "-ffunction-sections"
    "-fdata-sections"
    "-Wall"
    "-Wextra"
    "-g3"
    "-Og"
)

# Combine CPU and common flags
string(REPLACE ";" " " CPU_FLAGS_STR "${CPU_FLAGS}")
string(REPLACE ";" " " COMMON_FLAGS_STR "${COMMON_FLAGS}")

# Set the compiler flags
set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS_STR} ${COMMON_FLAGS_STR}")
set(CMAKE_CXX_FLAGS_INIT "${CPU_FLAGS_STR} ${COMMON_FLAGS_STR}")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS_STR} ${COMMON_FLAGS_STR}")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS_STR} -Wl,--gc-sections --specs=nosys.specs")

# Set the executable suffix
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
