cmake_minimum_required(VERSION 3.6)

project(libresources)

add_definitions(-g)

add_definitions(-D_LITTLE_ENDIAN_)

if(PLATFORM_WINDOWS)
    add_definitions(-D_OS_WINDOWS_ -D_WIN32_WINNT=0x0600)
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
					-D_GLIBCXX_USE_CXX11_ABI=0
					-std=c++14)
endif(PLATFORM_ESP8266)

include_directories(common/inc
                    export
                    .
                    ../base/export
                    ../osal/export
                    ../memory/export
                    ../debug/export
                    ../stack/export
                    ../network/export)
                    
set(COMMON_SRCS
				common/src/ResModuleInterface.cpp
                common/src/BinarySwitchResourceBase.cpp
                common/src/core/DeviceResource.cpp
                common/src/core/WellKnownResource.cpp
                common/src/security/AccessControlEntry.cpp
                common/src/security/AccessControlListRes.cpp
                common/src/security/Credential.cpp
                common/src/security/CredentialRes.cpp
                common/src/security/DeviceOwnerXferMethodRes.cpp
                common/src/security/ProvisioningStatusRes.cpp
                common/src/util/ResourceUtil.cpp
                )

add_library(resources STATIC ${COMMON_SRCS})
