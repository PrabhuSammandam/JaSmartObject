cmake_minimum_required(VERSION 3.6)

project(linux_server)

add_subdirectory(../../../code "${CMAKE_CURRENT_BINARY_DIR}/code_build")

include_directories(../../../code/base/export
                    ../../../code/memory/export
                    ../../../code/network/export
                    ../../../code/debug/export
                    ../../../code/stack/export
                    ../../../code/osal/export)

set(SRCS code/main.cpp code/BinarySwitchResource.cpp)

add_executable(linux_server ${SRCS})
target_link_libraries(linux_server stack osal network memory base pthread)
