# Security Camera Server 
# By Krishna Mani on 6/02/2019

cmake_minimum_required(VERSION 3.1) #CMAKE_CXX_STANDARD was introduced in 3.1

project(server)

file(GLOB_RECURSE SOURCES src/*.cpp)

set(CMAKE_CXX_STANDARD 11)

# find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
# find_library(AVFORMAT_LIB avformat)

# find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
# find_library(AVCODEC_LIB avcodec)

# find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
# find_library(AVUTIL_LIB avutil)
# # find_path(AVUTIL_INCLUDE_DIR libavutil/opt.h)
# # find_library(AVUTIL_LIB libavutil)

# # Output to bin/

find_package( OpenCV REQUIRED )

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

add_executable(server ${SOURCES})

include_directories(server include/)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

target_link_libraries(server PRIVATE ${AVFORMAT_LIB}  ${AVCODEC_LIB} ${AVUTIL_LIB} ${OpenCV_LIBS} PkgConfig::LIBAV)
target_include_directories(server PRIVATE ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${OpenCV_INCLUDE_DIR})
