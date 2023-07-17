cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(MyPerfect6502 LANGUAGES CXX)

#find_package(SDL2 REQUIRED)
#find_package(Vulkan REQUIRED)

#set(CMAKE_C_FLAGS "-DVK_ENABLE_BETA_EXTENSIONS -D_DEBUG")
set(CMAKE_CXX_FLAGS "-D_DEBUG -Wno-write-strings")

set(SOURCE_FILES perfect6502.cpp)
#set(IMGUI_PATH "3rdparty/imgui-1.89.2")
#set(IMGUI_PATH "3rdparty/imgui-docking")

#add_subdirectory(${IMGUI_PATH})
#add_subdirectory("system")
#add_subdirectory("renderer")
add_subdirectory("cbmbasic")
add_subdirectory("netlistsim")

#include_directories(${SDL2_INCLUDE_DIRS})
#include_directories(${Vulkan_INCLUDE_DIRS})
#include_directories(${IMGUI_PATH})
include_directories("cbmbasic")
include_directories("netlistsim")
include_directories(".")

add_executable(myperfect6502 ${SOURCE_FILES})
set_target_properties(myperfect6502 PROPERTIES LINKER_LANGUAGE CXX)
#target_link_libraries(myperfect6502)
