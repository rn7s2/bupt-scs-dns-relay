cmake_minimum_required(VERSION 3.25)
project(bupt-scs-dns-relay C)

set(CMAKE_C_STANDARD 11)

set(SOURCE
        src/main.c
        src/argsparser.c include/argsparser.h)
add_executable(bupt-scs-dns-relay ${SOURCE})

add_subdirectory(3rd-party/cargs)
target_link_libraries(bupt-scs-dns-relay cargs)

target_include_directories(bupt-scs-dns-relay PRIVATE include)
