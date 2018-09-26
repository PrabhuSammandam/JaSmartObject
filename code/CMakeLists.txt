cmake_minimum_required(VERSION 3.6)

option(PLATFORM_WINDOWS "Windows Platform to compile" OFF)
option(PLATFORM_LINUX "Linux Platform to compile" OFF)
option(PLATFORM_ESP8266 "esp8266 Platform to compile" OFF)

add_subdirectory(base)
add_subdirectory(memory)
add_subdirectory(osal)
add_subdirectory(network)
add_subdirectory(stack)