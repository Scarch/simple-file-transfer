cmake_minimum_required(VERSION 3.30)
project(simple-file-transfer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# Specifying Asio location
set(ASIO_INCLUDE_DIR "lib/asio/include" CACHE PATH "Path to Standalone Asio headers")
include_directories(${ASIO_INCLUDE_DIR})
add_compile_definitions(ASIO_STANDALONE)

include_directories(inc)

# Required for asynchronous work
find_package(Threads REQUIRED)

add_executable(simple-file-transfer
        main.cpp
        src/Server.cpp
        src/Client.cpp
        src/FileHandler.cpp
)

target_link_libraries(simple-file-transfer PRIVATE Threads::Threads)

# Windows support
if(WIN32)
    target_link_libraries(simple-file-transfer PRIVATE ws2_32 mswsock)
endif()