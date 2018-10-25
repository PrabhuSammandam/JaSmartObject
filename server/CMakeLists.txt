cmake_minimum_required(VERSION 3.6)

project(ja-coap-server)

if(ADAPTER STREQUAL IP)
    add_definitions(-D_IP_ADAPTER_)
endif()

if(PLATFORM_WINDOWS)
    add_definitions(-D_OS_WINDOWS_)
endif(PLATFORM_WINDOWS)

if(PLATFORM_LINUX)
    add_definitions(-D_OS_LINUX_)
endif(PLATFORM_LINUX)

add_subdirectory(../code "${CMAKE_CURRENT_BINARY_DIR}/code_build")

include_directories(../code/base/export
                    ../code/memory/export
                    ../code/network/export
                    ../code/debug/export
                    ../code/stack/export
                    ../code/osal/export
		    ~/.local/include)

add_definitions(-std=c++14)		  

set(SRCS main.cpp BinarySwitchResource.cpp)

add_executable(ja-coap-server ${SRCS})
target_link_libraries(ja-coap-server stack osal network memory base pthread)
