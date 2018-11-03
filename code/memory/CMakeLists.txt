cmake_minimum_required(VERSION 3.6)

project(libmemory)

if(PLATFORM_WINDOWS)
    add_definitions(-D_OS_WINDOWS_)
    include_directories(
                        port/windows/inc
                        )
endif(PLATFORM_WINDOWS)

if(PLATFORM_LINUX)
    add_definitions(-D_OS_LINUX_)
    include_directories(
                        port/linux/inc
                        )
endif(PLATFORM_LINUX)

if(PLATFORM_ESP8266)
	add_definitions(-D_OS_FREERTOS_
					-D__ets__
					-D__STDC_NO_ATOMICS__=1
					-DESP8266_RTOS
					-DFREERTOS_ARCH_ESP8266
					-DICACHE_FLASH
					-DLWIP_COMPAT_SOCKETS
					-D_GLIBCXX_USE_CXX11_ABI=0)
    include_directories(
                        port/freertos/inc
                        )
endif(PLATFORM_ESP8266)

add_definitions(-g)

include_directories(common/inc
                    export
                    port/common/inc
                    .)

set(COMMON_SRCS)
set(PORT_SRCS)

if(PLATFORM_WINDOWS)
    set(PORT_SRCS
        port/windows/src/MemAllocWindows.cpp
        )
endif(PLATFORM_WINDOWS)

if(PLATFORM_LINUX)
    set(PORT_SRCS
        port/linux/src/MemAllocLinux.cpp
        )
endif(PLATFORM_LINUX)

if(PLATFORM_ESP8266)
    set(PORT_SRCS
        port/freertos/src/MemAllocFreeRtos.cpp
        )
endif(PLATFORM_ESP8266)

set( COMMON_SRCS
        common/src/IMemAllocator.cpp
        )

add_library(memory STATIC ${COMMON_SRCS} ${PORT_SRCS} )