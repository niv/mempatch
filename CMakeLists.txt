cmake_minimum_required(VERSION 3.5)
project(mempatch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SUBHOOK_STATIC 1)

add_subdirectory(subhook)
include_directories(subhook)

include_directories(.)

add_library(mempatch STATIC
	Base
	Simple
	CallHook
	TrampolineHook
)
target_link_libraries(mempatch subhook)

add_executable(test_mempatch test.cpp)
target_link_libraries(test_mempatch mempatch)
