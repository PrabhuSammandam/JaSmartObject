CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(memory)

INCLUDE_DIRECTORIES(common/inc export port/common/inc port/windows/inc .)

ADD_LIBRARY(memory STATIC port/windows/src/MemAllocWindows.cpp common/src/IMemAllocator.cpp)