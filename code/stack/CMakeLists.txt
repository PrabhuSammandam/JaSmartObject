cmake_minimum_required(VERSION 3.6)

project(libstack)

add_definitions(-D_LITTLE_ENDIAN_)
add_definitions(-D_DEBUG_=1 -D_STACK_DEBUG_=1)

if(PLATFORM_WINDOWS)
    add_definitions(-D_OS_WINDOWS_ -D_WIN32_WINNT=0x0600)
    include_directories(port/windows/inc)
endif(PLATFORM_WINDOWS)

if(PLATFORM_LINUX)
    add_definitions(-D_OS_LINUX_)
    include_directories(port/linux/inc)
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

include_directories(common/inc
                    export
                    port/common/inc
                    .
                    ../base/export
                    ../memory/export
                    ../debug/export
                    ../osal/export
                    ../network/export
                    ../resources/export)
                    
set(COMMON_SRCS
            common/src/BaseInteraction.cpp
            common/src/ClientInteraction.cpp
            common/src/InteractionStore.cpp
            common/src/MsgStack.cpp
            common/src/ResPropValue.cpp
            common/src/TokenProvider.cpp
            common/src/BaseResource.cpp
            common/src/MessageIdProvider.cpp
            common/src/QueryContainer.cpp
            common/src/ServerInteraction.cpp
            common/src/BlockTransferStatus.cpp
            common/src/Exchange.cpp
            common/src/MessageSender.cpp
            common/src/ResourceMgr.cpp
            common/src/MulticastClientInteraction.cpp
common/src/SoStackFacade.cpp
            )
            
set (CBOR_SRCS
    common/src/cbor/CborCodec.cpp
    common/src/cbor/CborEncoding.cpp
    common/src/cbor/CborItem.cpp
    common/src/cbor/CborItemBuilder.cpp
    common/src/cbor/CborPrettyPrinter.cpp
    common/src/cbor/CborStreaming.cpp)            

add_library(stack STATIC ${COMMON_SRCS} ${CBOR_SRCS})
