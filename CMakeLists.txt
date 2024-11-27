cmake_minimum_required(VERSION 3.29)
project(SP1 C)

set(CMAKE_C_STANDARD 17)

add_executable(SP1
        main.c
        btree.h
        btree.c
        database.h
        database.c)
