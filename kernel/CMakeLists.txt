cmake_minimum_required (VERSION 3.14.5)
PROJECT (kernel)

INCLUDE_DIRECTORIES (include)
INCLUDE_DIRECTORIES (../include_common)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include-fixed)

ENABLE_LANGUAGE (ASM_NASM)

FILE (GLOB_RECURSE kernel_C_SOURCES "**/*.c")
FILE (GLOB_RECURSE kernel_CPP_SOURCES "**/*.cpp")
FILE (GLOB_RECURSE kernel_ASM_SOURCES "**/*.asm")

#FILE (GLOB_RECURSE kernel_ARCH_C_SOURCES "arch/x86/*.c")
#FILE (GLOB_RECURSE kernel_ARCH_CPP_SOURCES "arch/x86/*cpp")

add_library(kernel.elf STATIC
			${kernel_C_SOURCES} 
			${kernel_CPP_SOURCES} 
			#${kernel_ARCH_C_SOURCES} 
			#${kernel_ARCH_CPP_SOURCES} 
			${kernel_ASM_SOURCES})

message("Kernel Binary bin ${PROJECT_BINARY_DIR}")
add_custom_command(TARGET kernel.elf POST_BUILD COMMAND ${CMAKE_C_LINKER} -ffreestanding -fno-rtti -fno-exceptions -nostdlib -T${CMAKE_CURRENT_LIST_DIR}/link.ld -o kernel.elf ${PROJECT_BINARY_DIR}/libkernel.elf.a)