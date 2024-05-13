cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(INCLUDE_DIR include)
# Sanitizer option
option(ENABLE_SANITIZERS "Enable Sanitizers (UBSan, ASan, LSAN)" OFF)

# Conditionally add sanitizer flags
if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=undefined,address,leak)
    add_link_options(-fsanitize=undefined,address,leak)
endif()

set(SOURCE_FILES src/server.cpp src/response.cpp src/request.cpp)

find_package(ZLIB REQUIRED)
add_executable(server ${SOURCE_FILES})
target_include_directories(server PRIVATE ${INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(server pthread ${ZLIB_LIBRARIES})
