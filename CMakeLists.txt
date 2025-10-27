cmake_minimum_required(VERSION 3.5)

project( nucleus_tests )

set (CMAKE_C_STANDARD 11 )
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/ )
set (CMAKE_BUILD_TYPE Debug )

include_directories( tests.out PRIVATE Unity )
include_directories( tests.out PRIVATE Unity/src )
include_directories( tests.out PRIVATE src )
include_directories( tests.out PRIVATE boilerplate/src )
include_directories( tests.out PRIVATE tests )
include_directories( tests.out PRIVATE common )

add_executable( tests.out
                common/mqtt.c
                boilerplate/src/fifo_base.c
                boilerplate/src/pq_base.c
                tests/tests.c
                tests/mqtt_tests.h
                tests/mqtt_tests.c
                Unity/src/unity.c
                Unity/src/unity.h
                Unity/src/unity_internals.h ) 
target_compile_options( tests.out
                        PUBLIC
                        -Wfatal-errors
                        #-Wall
                        #-Wextra
                        -Werror
                        #-Wpedantic
                        #-Wpointer-arith
                        -g
                        -DUNIT_TESTS
                        -DUNITY_OUTPUT_COLOR )

