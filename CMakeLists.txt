cmake_minimum_required(VERSION 3.25)
project(bupt-scs-dns-relay C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCE
        src/main.c
        src/argsparser.c include/argsparser.h
        src/util.c include/util.h
        3rd-party/trie/trie.c 3rd-party/trie/trie.h)
add_executable(bupt-scs-dns-relay ${SOURCE})

add_subdirectory(3rd-party/cargs)
target_link_libraries(bupt-scs-dns-relay cargs)

target_include_directories(bupt-scs-dns-relay PRIVATE include)
target_include_directories(bupt-scs-dns-relay PRIVATE 3rd-party/trie)
