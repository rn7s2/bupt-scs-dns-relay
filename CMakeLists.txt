cmake_minimum_required(VERSION 3.25)
project(bupt-scs-dns-relay C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

set(SOURCE
        src/main.c
        src/args.c include/args.h
        src/cache.c include/cache.h
        src/filerules.c include/filerules.h
        src/handler.c include/handler.h
        src/dns.c include/dns.h
        src/logger.c include/logger.h
        src/util.c include/util.h
        src/trie.c include/trie.h)
add_executable(bupt-scs-dns-relay ${SOURCE})

target_link_libraries(bupt-scs-dns-relay PkgConfig::deps)

add_subdirectory(3rd-party/cargs)
target_link_libraries(bupt-scs-dns-relay cargs)

target_include_directories(bupt-scs-dns-relay PRIVATE include)
