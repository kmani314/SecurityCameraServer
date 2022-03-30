# Security Camera Server 
# By Krishna Mani on 6/02/2019

cmake_minimum_required(VERSION 3.1) #CMAKE_CXX_STANDARD was introduced in 3.1

project(server)

file(GLOB_RECURSE SOURCES src/*.cpp)

set(CMAKE_CXX_STANDARD 11)

# # Output to bin/
find_package( OpenCV REQUIRED )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

add_executable(server ${SOURCES})

include_directories(server include/)

SET(CMAKE_CXX_FLAGS)

target_link_libraries(server PRIVATE)
target_include_directories(server PRIVATE ${OpenCV_INCLUDE_DIR})
