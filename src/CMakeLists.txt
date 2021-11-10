#AUX_SOURCE_DIRECTORY(./ LIBNAP_SRCS)


SET (LIBNAP_SRCS "./btring.cpp" "./nap_common.cpp" )

if (BUILD_HASH)
    LIST(APPEND LIBNAP_SRCS "./hash.cpp" )
    LIST(APPEND LIBNAP_SRCS "./trans.cpp" )
endif()

if (BUILD_JSON)
    LIST(APPEND LIBNAP_SRCS "./json.cpp" )
endif()

if (BUILD_NET)
    LIST(APPEND LIBNAP_SRCS "./net.cpp" )
endif()

if (BUILD_THREADPOOL)
    LIST(APPEND LIBNAP_SRCS "./threadpool.cpp" )
endif()

if (BUILD_AES)
    LIST(APPEND LIBNAP_SRCS "./aes.cpp" )
endif()

ADD_LIBRARY( nap STATIC ${LIBNAP_SRCS} )


if (PLATFORM MATCHES "linux")
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
    SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
    INSTALL(TARGETS nap
        ARCHIVE DESTINATION lib/ )

    INSTALL(
        DIRECTORY ./
        DESTINATION include/libnap
        FILES_MATCHING PATTERN "*.h")

endif()