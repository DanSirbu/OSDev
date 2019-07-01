# CMakefile from https://github.com/phaser/phOS/blob/master/CMakeLists.txt

cmake_minimum_required (VERSION 3.14.5)
PROJECT (libc)

INCLUDE_DIRECTORIES (include)
INCLUDE_DIRECTORIES (../include_common)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include-fixed)

SET (CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-rtti -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector")
SET (CMAKE_C_FLAGS "-m32 -ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector -fno-pic")
SET (CMAKE_ASM_NASM_OBJECT_FORMAT "elf")

ENABLE_LANGUAGE (ASM_NASM)

FILE (GLOB_RECURSE libc_C_SOURCES "**/*.c")
FILE (GLOB_RECURSE libc_CPP_SOURCES "**/*.cpp")
FILE (GLOB_RECURSE libc_ASM_SOURCES "**/*.asm")

add_library(c STATIC
			${libc_C_SOURCES} 
			${libc_CPP_SOURCES} 
			${libc_ASM_SOURCES})