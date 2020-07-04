cmake_minimum_required(VERSION 3.13)

add_library(wingetopt STATIC src/getopt.c)
target_include_directories(wingetopt
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

