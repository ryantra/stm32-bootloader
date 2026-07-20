# Toolchain file for the ARM GCC bare-metal cross-compiler.
# Usage: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "")
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size    CACHE INTERNAL "")

# Bare-metal: don't try to link a full executable during the compiler check.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
