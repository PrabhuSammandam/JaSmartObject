cmake_minimum_required(VERSION 3.6)

project(win_coap_server)

add_subdirectory(../../../code "${CMAKE_CURRENT_BINARY_DIR}/code_build")

include_directories(../../../code/base/export
                    ../../../code/memory/export
                    ../../../code/network/export
                    ../../../code/debug/export
                    ../../../code/stack/export
                    ../../../code/osal/export)

set(SRCS
    code/main.cpp)

add_executable(win_coap_server ${SRCS})
target_link_libraries(win_coap_server stack osal network memory base Ws2_32 Iphlpapi Bcrypt)