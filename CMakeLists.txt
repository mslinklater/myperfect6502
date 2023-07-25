cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(MyPerfect6502 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-D_DEBUG -g -O0 -std=gnu++20 -Wno-write-strings -Wno-unused-result")
#set(CMAKE_CXX_FLAGS "-D_DEBUG -g -pg -O0 -Wno-write-strings -Wno-unused-result")
#set(CMAKE_CXX_FLAGS "-D_DEBUG -g -O3 -Wno-write-strings -Wno-unused-result")

set(SOURCE_FILES perfect6502.cpp netlist_sim.cpp debug.cpp)

add_subdirectory("cbmbasic")
add_subdirectory("netlistsim")
add_subdirectory("netlist_6502")

include_directories("cbmbasic")
include_directories("netlistsim")
include_directories("netlist_6502")
include_directories(".")

add_executable(myperfect6502 ${SOURCE_FILES})
set_target_properties(myperfect6502 PROPERTIES LINKER_LANGUAGE CXX)
