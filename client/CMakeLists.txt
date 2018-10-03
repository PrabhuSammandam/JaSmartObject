cmake_minimum_required(VERSION 3.6)

project(ja-coap-client)

add_subdirectory(../code "${CMAKE_CURRENT_BINARY_DIR}/code_build")

include_directories(../code/base/export
                    ../code/memory/export
                    ../code/network/export
                    ../code/debug/export
                    ../code/stack/export
                    ../code/osal/export
		    ~/.local/include)

add_definitions(-std=c++14)		  

set(SRCS Client.cpp SimpleClient.cpp)

add_executable(ja-coap-client ${SRCS})
target_link_libraries(ja-coap-client stack osal network memory base pthread)
