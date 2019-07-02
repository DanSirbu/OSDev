# CMakefile from https://github.com/phaser/phOS/blob/master/CMakeLists.txt

cmake_minimum_required (VERSION 3.14.5)
PROJECT (libc C ASM_NASM)

set(CMAKE_NASM_LINK_EXECUTABLE "ld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CAN_USE_ASSEMBLER TRUE)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM${ASM_DIALECT}_SOURCE_FILE_EXTENSIONS "s;S;asm;nasm")

enable_language(ASM_NASM)


#set(CMAKE_ASM_NASM_LINK_EXECUTABLE "ld <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_ASM_NASM_FLAGS_DEBUG "-g -Fdwarf -f elf32")

#add_compile_options(
#    "$<$<COMPILE_LANGUAGE:ASM_NASM>:-f $<IF:$<BOOL:$<TARGET_PROPERTY:NASM_OBJ_FORMAT>>, \
#    $<TARGET_PROPERTY:NASM_OBJ_FORMAT>, ${CMAKE_ASM_NASM_OBJECT_FORMAT}>>"
#)

INCLUDE_DIRECTORIES (include)
INCLUDE_DIRECTORIES (../include_common)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include-fixed)

SET (CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-rtti -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector")
SET (CMAKE_C_FLAGS "-m32 -ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector -fno-pic")


set(PROJECT_C_SOURCES ${PROJECT_NAME}_C_SOURCES)
set(PROJECT_CPP_SOURCES ${PROJECT_NAME}_CPP_SOURCES)
set(PROJECT_S_SOURCES ${PROJECT_NAME}_S_SOURCES)
set(PROJECT_NASM_SOURCES ${PROJECT_NAME}_NASM_SOURCES)

FILE (GLOB_RECURSE PROJECT_C_SOURCES "**/*.c")
FILE (GLOB_RECURSE PROJECT_CPP_SOURCES "**/*.cpp")
FILE (GLOB_RECURSE PROJECT_S_SOURCES "*.s")
FILE (GLOB_RECURSE PROJECT_NASM_SOURCES "*.nasm")

add_library(c STATIC
			${PROJECT_C_SOURCES} 
			#${PROJECT_CPP_SOURCES} 
			#${PROJECT_S_SOURCES}
            ${PROJECT_NASM_SOURCES}
            )