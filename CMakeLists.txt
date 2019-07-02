# CMakefile from https://github.com/phaser/phOS/blob/master/CMakeLists.txt

cmake_minimum_required (VERSION 3.14.5)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/toolchain/local/bin/i686-pc-corax-gcc)
set(CMAKE_C_LINKER ${CMAKE_CURRENT_SOURCE_DIR}/toolchain/local/bin/i686-pc-corax-gcc)

SET (CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-rtti -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector")
SET (CMAKE_C_FLAGS "-m32 -ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector -fno-pic")
SET (CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostdinc -fno-common")

message("Binary bin: ${CMAKE_BINARY_DIR}")

PROJECT (global_make)

add_subdirectory(kernel)
add_subdirectory(libc)
add_subdirectory(apps)

add_custom_target(os.iso ALL
                  COMMAND bash make-iso.sh
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(run
                 COMMAND qemu-system-i386 -no-shutdown -no-reboot -s -m 512M -serial file:serial.log -drive format=raw,file=os.iso -vga std
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 )

add_dependencies(run os.iso)