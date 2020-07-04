cmake_minimum_required(VERSION 3.13)

# gtest-all.cc includes all other sources
add_library(gtest STATIC src/gtest-all.cc)

target_include_directories(gtest
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(gtest PUBLIC GTEST_HAS_TR1_TUPLE=0)
