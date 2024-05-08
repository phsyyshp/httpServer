cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)
set(CMAKE_CXX_STANDARD 20) # Enable the C++20 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(INCLUDE_DIR include)

set(SOURCE_FILES src/server.cpp
src/response.cpp
src/get.cpp
src/request.cpp)

add_executable(server ${SOURCE_FILES})
target_include_directories(server PRIVATE ${INCLUDE_DIR})
target_link_libraries(server pthread)
