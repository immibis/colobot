cmake_minimum_required(VERSION 3.13)

add_library(localename STATIC localename.c)
target_include_directories(localename PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
