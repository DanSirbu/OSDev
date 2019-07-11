cmake_minimum_required (VERSION 3.14.5)
PROJECT (apps)

INCLUDE_DIRECTORIES (../libc/include)
INCLUDE_DIRECTORIES (../include_common)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include)
INCLUDE_DIRECTORIES (../toolchain/local/lib/gcc/i686-pc-corax/8.2.0/include-fixed)

SET (CMAKE_CXX_FLAGS "-m32 -ffreestanding -fno-rtti -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector")
SET (CMAKE_C_FLAGS "-m32 -ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-common -fno-stack-protector -fno-pic")

set(PROJECT_C_SOURCES ${PROJECT_NAME}_C_SOURCES)
set(PROJECT_CPP_SOURCES ${PROJECT_NAME}_CPP_SOURCES)


FILE (GLOB PROJECT_C_SOURCES "*.c")
FILE (GLOB PROJECT_CPP_SOURCES "*.cpp")

message("Apps: ${PROJECT_C_SOURCES}")

foreach(appFile ${PROJECT_C_SOURCES})
get_filename_component(appFileTarget ${appFile} NAME_WE)
add_executable(${appFileTarget} ${appFile})
target_link_libraries(${appFileTarget} c)
target_link_libraries(${appFileTarget} gcc)

set_target_properties(${appFileTarget} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/built_apps")

list(APPEND TARGETS_ ${appFileTarget})
endforeach()

# Create ramfs target     
add_custom_target(additional_apps
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/binaries/* "${CMAKE_BINARY_DIR}/built_apps" 2>/dev/null || :
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(ramfs.img ALL
                  COMMAND make -C ramfs
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ramfs.img dependencies
add_dependencies(ramfs.img additional_apps)
foreach(target ${TARGETS_})
add_dependencies(ramfs.img ${target})
endforeach()