cmake_minimum_required(VERSION 3.6)

project(libosal)

add_definitions(-D_LITTLE_ENDIAN_ -D_WIN32_WINNT=0x0600)

if(PLATFORM_WINDOWS)
    add_definitions(-D_OS_WINDOWS_ )
    include_directories(
                        port/windows/inc
                        )
endif()

if(PLATFORM_LINUX)
    add_definitions(-D_OS_LINUX_ )
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
						${ESP8266_SDK_INCLUDES}
                        )
endif(PLATFORM_ESP8266)

include_directories(common/inc
                    export
                    port/common/inc
                    .
                    ../base/export
                    ../memory/export
                    ../debug/export)
                    
set(COMMON_SRCS
        common/src/OsalMgr.cpp
        common/src/OsalRandom.cpp
        common/src/OsalTimer.cpp
        common/src/OsalTimerMgr.cpp
        common/src/TaskBase.cpp )
              
set(PORT_SRCS)

if(PLATFORM_WINDOWS)
set( PORT_SRCS 
        port/windows/src/ConditionImplWindows.cpp
        port/windows/src/MutexImplWindows.cpp
        port/windows/src/OsalBuilderImplWindows.cpp
        port/windows/src/OsalRandomImplWindows.cpp
        port/windows/src/OsTimeWindows.cpp
        port/windows/src/SemaphoreImplWindows.cpp
        port/windows/src/TaskImplWindows.cpp)
endif()      

if(PLATFORM_LINUX)
set( PORT_SRCS 
        port/linux/src/MutexImplLinux.cpp
        port/linux/src/OsalBuilderImplLinux.cpp
        port/linux/src/OsalRandomImplLinux.cpp
        port/linux/src/OsTimeLinux.cpp
        port/linux/src/SemaphoreImplLinux.cpp
        port/linux/src/TaskImplLinux.cpp
        )
endif(PLATFORM_LINUX)      

if(PLATFORM_ESP8266)
set( PORT_SRCS 
        port/freertos/src/MutexImplFreertos.cpp
        port/freertos/src/OsalBuilderImplFreertos.cpp
        port/freertos/src/OsalRandomImplFreertos.cpp
        port/freertos/src/OsTimeFreertos.cpp
        port/freertos/src/SemaphoreImplFreertos.cpp
        port/freertos/src/TaskImplFreertos.cpp
        )
endif(PLATFORM_ESP8266)

add_library(osal STATIC ${COMMON_SRCS} ${PORT_SRCS} )