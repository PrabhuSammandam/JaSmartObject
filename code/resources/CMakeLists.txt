cmake_minimum_required(VERSION 3.6)

project(libresources)

add_definitions(-g)

include_directories(common/inc
                    export
                    .
                    ../base/export
                    ../memory/export
                    ../debug/export
                    ../stack/export
                    ../network/export)
                    
set(COMMON_SRCS
                common/src/BinarySwitchResourceBase.cpp)

add_library(resources STATIC ${COMMON_SRCS})
