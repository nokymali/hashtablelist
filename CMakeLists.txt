cmake_minimum_required(VERSION 4.2)
project(hashtablelist C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(hashtablelist
        hashlist.c
        hashlist.h
        main.c
        demo_data.h)
