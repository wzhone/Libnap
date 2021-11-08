CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(Libnap)
set (CMAKE_CXX_STANDARD 17)


SET(CMAKE_BUILD_TYPE "Debug")

option(BUILD_AES "build aes module" ON)
if (BUILD_AES)
    add_definitions(-DBUILD_AES)
endif()

option(BUILD_HASH "build hash module" ON)
if (BUILD_HASH)
    add_definitions(-DBUILD_HASH)
endif()

option(BUILD_JSON "build json module" ON)
if (BUILD_JSON)
    add_definitions(-DBUILD_JSON)
endif()

option(BUILD_NET "build net module" ON)
if (BUILD_NET)
    add_definitions(-DBUILD_NET)
endif()

option(BUILD_THREADPOOL "build thread pool module" ON)
if (BUILD_THREADPOOL)
    add_definitions(-DBUILD_THREADPOOL)
endif()

option(BUILD_TEST "build test" ON)
if (BUILD_TEST)
    ADD_SUBDIRECTORY( ./test )
endif()

ADD_SUBDIRECTORY( ./src )


