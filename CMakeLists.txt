cmake_minimum_required(VERSION 2.8.10)
project(echo_server C CXX)

option(BRPC_LINK_SO "Whether brpc are linked dynamically" OFF)

include(FindThreads)
#include(FindProtobuf)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

# find_path(GPERFTOOLS_INCLUDE_DIR NAMES gperftools/heap-profiler.h)
# find_library(GPERFTOOLS_LIBRARIES NAMES tcmalloc_and_profiler)
# include_directories(${GPERFTOOLS_INCLUDE_DIR})

# find_path(BRPC_INCLUDE_PATH NAMES brpc/server.h)
# if(BRPC_LINK_SO)
#     find_library(BRPC_LIB NAMES brpc)
# else()
#     find_library(BRPC_LIB NAMES libbrpc.a brpc)
# endif()
# if((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
#     message(FATAL_ERROR "Fail to find brpc")
# endif()
# include_directories(${BRPC_INCLUDE_PATH})

# find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
# find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
# if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
#     message(FATAL_ERROR "Fail to find gflags")
# endif()
# include_directories(${GFLAGS_INCLUDE_PATH})

# execute_process(
#     COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
#     OUTPUT_VARIABLE GFLAGS_NS
# )
# if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
#     execute_process(
#         COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
#         OUTPUT_VARIABLE GFLAGS_NS
#     )
# endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
    if(NOT HAVE_CLOCK_GETTIME)
        set(DEFINE_CLOCK_GETTIME "-DNO_CLOCK_GETTIME_IN_MAC")
    endif()
endif()

set(CMAKE_CPP_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBRPC_ENABLE_CPU_PROFILER")

if(CMAKE_VERSION VERSION_LESS "3.1.3")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
# find_library(LEVELDB_LIB NAMES leveldb)
# if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
#     message(FATAL_ERROR "Fail to find leveldb")
# endif()
# include_directories(${LEVELDB_INCLUDE_PATH})

find_library(SSL_LIB NAMES ssl)
if (NOT SSL_LIB)
    message(FATAL_ERROR "Fail to find ssl")
endif()

find_library(CRYPTO_LIB NAMES crypto)
if (NOT CRYPTO_LIB)
    message(FATAL_ERROR "Fail to find crypto")
endif()

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${SSL_LIB}
    ${CRYPTO_LIB}
    dl
    event
    ${GPERFTOOLS_LIBRARIES}
    )

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
        pthread
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreData"
        "-framework CoreText"
        "-framework Security"
        "-framework Foundation"
        "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
        "-Wl,-U,_ProfilerStart"
        "-Wl,-U,_ProfilerStop")
endif()

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/eventthread.cpp)
    # ${CMAKE_SOURCE_DIR}/src/delayed_log_base.cpp
    # ${CMAKE_SOURCE_DIR}/src/event_log_manager.cpp
    # ${CMAKE_SOURCE_DIR}/src/frame_queue.cpp
    # ${CMAKE_SOURCE_DIR}/src/hls_min_js.cpp
    # ${CMAKE_SOURCE_DIR}/src/http_streaming_service.cpp
    # ${CMAKE_SOURCE_DIR}/src/media_server.cpp
    # ${CMAKE_SOURCE_DIR}/src/rtmp_forward_service.cpp
    # ${CMAKE_SOURCE_DIR}/src/ts_queue.cpp
    # ${CMAKE_SOURCE_DIR}/src/util.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/bin)
include_directories(${CMAKE_SOURCE_DIR}/src)

link_libraries(event pthread)
add_executable(${PROJECT_NAME} ${SOURCES} )
# file(COPY ${CMAKE_SOURCE_DIR}/scripts/
#      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/output/bin/
#      FILES_MATCHING
#      PATTERN "*.sh"
#      )
