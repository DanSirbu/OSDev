cmake_minimum_required (VERSION 3.14.5)

set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/toolchain/local/bin/i686-pc-corax-gcc)
set(CMAKE_C_LINKER ${CMAKE_CURRENT_SOURCE_DIR}/toolchain/local/bin/i686-pc-corax-gcc)

SET (CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-rtti -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector")
SET (CMAKE_C_FLAGS "-m32 -ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector -fno-pic")
SET (CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostdinc -fno-common")

message("Binary bin: ${CMAKE_BINARY_DIR}")

PROJECT (global_make)

add_subdirectory(kernel)
add_subdirectory(libc)