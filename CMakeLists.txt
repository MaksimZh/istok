cmake_minimum_required(VERSION 3.15)
project(Istok)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(test)
add_subdirectory(tools)
add_subdirectory(logging)
add_subdirectory(ecs)
add_subdirectory(gui)
add_subdirectory(playground)
