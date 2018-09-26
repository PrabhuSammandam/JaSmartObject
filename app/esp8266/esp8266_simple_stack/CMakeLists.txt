cmake_minimum_required(VERSION 3.6)

project(esp8266_server C CXX)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

add_definitions( -D__ets__
					-D__STDC_NO_ATOMICS__=1
					-DESP8266_RTOS
					-DFREERTOS_ARCH_ESP8266
					-DICACHE_FLASH
					-DLWIP_COMPAT_SOCKETS
					-D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory(../../../code "${CMAKE_CURRENT_BINARY_DIR}/code_build")

include_directories(code
						code/include
						../../../code/base/export
						../../../code/debug/export
						../../../code/memory/export
						../../../code/osal/export
						../../../code/network/export
						../../../code/stack/export
						${ESP8266_SDK_INCLUDES})

set (SRCS
	code/user/cplusplus.cpp
	code/user/user_main.cpp
	code/user/resources/SwitchResource.cpp
	code/user/common/ja_app_init.c
	code/user/common/ja_gpio.c
	code/user/common/ja_gpio_led.c
	code/user/common/ja_key_mgr.c
	)

add_executable(esp8266_server ${SRCS})
cotire(esp8266_server)
target_link_libraries(esp8266_server
						stack
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
						hal )
						
add_custom_target(memsize COMMAND $ENV{ESP8266_TOOLS_ROOT}/MemAnalyzer.exe $ENV{ESP8266_COMPILER_ROOT}/xtensa-lx106-elf-objdump.exe esp8266_server.elf)					
add_custom_target(flash COMMAND $ENV{ESP8266_TOOLS_ROOT}/esptool2.exe -quiet -bin -boot0 -4096 -qio -40 esp8266_server.elf 0x00000.bin .text .data .rodata
							COMMAND $ENV{ESP8266_TOOLS_ROOT}/esptool2.exe -quiet -lib esp8266_server.elf 0x20000.bin
							COMMAND $ENV{ESP8266_TOOLS_ROOT}/esptool-ck.exe -v -cd ck -cb 921600 -cp COM4 -ca 0x00000 -cf 0x00000.bin -ca 0x20000 -cf 0x20000.bin)