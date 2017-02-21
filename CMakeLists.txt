cmake_minimum_required(VERSION 3.6)
project(myfind)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra -Wstrict-prototypes -fexceptions \
        -Wno-variadic-macros -pedantic -fno-common -O3 -Wno-long-long -std=gnu11 ")

set(SOURCE_FILES myfind.c)
add_executable(myfind ${SOURCE_FILES})