cmake_minimum_required(VERSION 3.6)

project(esp8266_net_app C CXX)

add_definitions(-D__ets__
                -D__STDC_NO_ATOMICS__=1
                -DESP8266_RTOS
                -DFREERTOS_ARCH_ESP8266
                -DICACHE_FLASH
                -DLWIP_COMPAT_SOCKETS
                -D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory(../../../code/base "${CMAKE_CURRENT_BINARY_DIR}/base_build")
add_subdirectory(../../../code/memory
                 "${CMAKE_CURRENT_BINARY_DIR}/memory_build")
add_subdirectory(../../../code/osal "${CMAKE_CURRENT_BINARY_DIR}/osal_build")
add_subdirectory(../../../code/network
                 "${CMAKE_CURRENT_BINARY_DIR}/network_build")

include_directories(code
                    code/include
                    ../../../code/base/export
                    ../../../code/debug/export
                    ../../../code/memory/export
                    ../../../code/osal/export
                    ../../../code/network/export
                    ${ESP8266_SDK_INCLUDES})

set(SRCS
    code/user/cplusplus.cpp
    code/user/user_main.cpp
    code/user/adapter_task.cpp
    code/user/wifi_task.cpp)

add_executable(esp8266_net_app ${SRCS})
target_link_libraries(esp8266_net_app
                      -Wl,--start-group
                      network
                      mbedtls
                      gcc
                      phy
                      pp
                      net80211
                      wpa
                      crypto
                      main
                      freertos
                      lwip
                      smartconfig
                      espnow
                      mirom
                      nopoll
                      pwm
                      espconn
                      json
                      openssl
                      spiffs
                      wps
                      memory
                      osal
                      base
                      stdc++
                      supc++
                      c
                      minic
                      hal
                      -Wl,--end-group)
