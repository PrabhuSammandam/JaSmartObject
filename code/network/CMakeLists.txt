cmake_minimum_required(VERSION 3.6)

project(libnetwork)

add_definitions(-D_LITTLE_ENDIAN_ -D_WIN32_WINNT=0x0600)
add_definitions(-D_DEBUG_ -D_NETWORK_DEBUG_)

if(ADAPTER STREQUAL IP)
	message("IP Adapter selected")
    add_definitions(-D_IP_ADAPTER_)
endif()

if(PLATFORM_WINDOWS)
	#message("Windows option")
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
                        port/esp8266/inc
						${ESP8266_SDK_INCLUDES}
                        )
endif(PLATFORM_ESP8266)

include_directories(common/inc
                    export
                    port/common/inc
                    .
                    ../base/export
                    ../memory/export
                    ../debug/export
                    ../osal/export)
                    
set(COMMON_SRCS
                common/src/adapter_mgr_impl.cpp
                common/src/config_mgr.cpp
                common/src/end_point.cpp
                common/src/ip_adapter_base.cpp
                common/src/interface_addr.cpp
                common/src/platform_factory.cpp
                common/src/coap/coap_block_option.cpp
                common/src/coap/coap_msg.cpp
                common/src/coap/coap_msg_codec.cpp
                common/src/coap/coap_option.cpp
                common/src/coap/coap_option_set.cpp
                common/src/coap/coap_options_set.cpp
                common/src/coap/CoapMsgPrinter.cpp)
                
set(PORT_SRCS)                

if(PLATFORM_WINDOWS)
set( PORT_SRCS 
        port/windows/src/interface_monitor_win.cpp
        port/windows/src/ip_adapter_win.cpp
        port/windows/src/platform_factory_win.cpp
        port/windows/src/udp_socket_win.cpp)
endif(PLATFORM_WINDOWS)

if(PLATFORM_LINUX)
set( PORT_SRCS 
        port/linux/src/ip_adapter_lnx.cpp
        port/linux/src/platform_factory_lnx.cpp
        port/linux/src/udp_socket_lnx.cpp
        )
endif(PLATFORM_LINUX)

if(PLATFORM_ESP8266)
set( PORT_SRCS 
        port/esp8266/src/interface_monitor_esp8266.cpp
        port/esp8266/src/ip_adapter_esp8266.cpp
        port/esp8266/src/platform_factory_esp8266.cpp
		port/esp8266/src/udp_socket_esp8266.cpp
        )
endif(PLATFORM_ESP8266)

add_library(network STATIC ${COMMON_SRCS} ${PORT_SRCS} )
#target_compile_features(network PRIVATE cxx_std_14)
