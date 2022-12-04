cmake_minimum_required(VERSION 3.23)
project(OSPratice C)

set(CMAKE_C_STANDARD 11)

include_directories(exp4)

add_executable(OSPratice
        exp4/list.c
        exp4/list.h
        exp4/myfs.c
        exp4/myfs.h
        exp4/block.h
        exp4/block.c
        exp4/file.c
        exp4/file.h
        exp4/types.c
        exp4/types.h)
