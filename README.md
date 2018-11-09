# JaSmartObject

Build commands

Windows
Following are the libraries used in the windows platform
1. ws2_32
2. iphlpapi
3. bcrypt

CMAKE

Linux and Windows
$ cmake -DADAPTER=IP -DPLATFORM_LINUX=ON

esp8266
$cmake -G "MSYS Makefiles" -DADAPTER=IP -DPLATFORM_ESP8266=ON -DCMAKE_TOOLCHAIN_FILE=/home/psammand/cmake/toolchain.ESP8266.cmake ..
