CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(Libnap)

# use C++ Standard 17
SET (CMAKE_CXX_STANDARD 17)

# only works on single-configuration generators 
SET (CMAKE_BUILD_TYPE "Release")

# only works on multi-config generators
SET (CMAKE_CONFIGURATION_TYPES "Debug;Release")

# platform
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET (PLATFORM , "linux")
    MESSAGE(STATUS  "Platform: ${CMAKE_SYSTEM_NAME}")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET (PLATFORM , "windows")
    MESSAGE(STATUS  "Platform: ${CMAKE_SYSTEM_NAME}")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	MESSAGE(FATAL_ERROR "support platform: ${CMAKE_SYSTEM_NAME}")
ENDIF ()

# specific module switch
include(CMakeDependentOption)

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

CMAKE_DEPENDENT_OPTION(BUILD_AES "build aes module" ON "BUILD_HASH;" OFF)
if (BUILD_AES)
    add_definitions(-DBUILD_AES)
endif()

option(BUILD_TEST "build test" ON)
if (BUILD_TEST)
    ADD_SUBDIRECTORY( ./test )
endif()

ADD_SUBDIRECTORY( ./src ./libnap )
