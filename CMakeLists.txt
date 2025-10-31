cmake_minimum_required(VERSION 4.0)
project(tarea_03_btree_a)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(tarea_03_btree_a
        btree.h
        main.cpp
        node.h
        tester.h)
