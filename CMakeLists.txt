cmake_minimum_required(VERSION 3.25)
project(bupt-scs-dns-relay C)

set(CMAKE_C_STANDARD 11)

set(SOURCE
        src/main.c
        src/process.c include/process.h
        src/ruleconf.c include/ruleconf.h)
add_executable(bupt-scs-dns-relay ${SOURCE})

target_include_directories(bupt-scs-dns-relay PRIVATE include)
