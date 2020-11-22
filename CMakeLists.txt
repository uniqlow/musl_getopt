cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

project("musl_getopt" C)

add_library(musl_getopt STATIC src/musl_getopt.c src/musl_getopt_long.c)
target_include_directories(musl_getopt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
