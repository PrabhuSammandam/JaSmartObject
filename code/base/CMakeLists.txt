cmake_minimum_required(VERSION 3.6)

project(libbase)

if(PLATFORM_WINDOWS)
  add_definitions(-D_OS_WINDOWS_)
endif(PLATFORM_WINDOWS)

if(PLATFORM_LINUX)
  add_definitions(-D_OS_LINUX_)
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

endif(PLATFORM_ESP8266)

add_definitions(-D_LITTLE_ENDIAN_ -g)

set(SRCS
    common/src/byte_array.cpp
    common/src/Hash.cpp
    common/src/ip_addr.cpp
    common/src/ip_socket_addr.cpp
    common/src/Uuid.cpp)

include_directories(common/inc export ../memory/export)

add_library(base STATIC ${SRCS})

if(PLATFORM_ESP8266)
  target_compile_options(base PRIVATE)
endif(PLATFORM_ESP8266)
