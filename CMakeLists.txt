cmake_minimum_required(VERSION 3.28)
project(Istok)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(ecs)
add_subdirectory(gui)
add_subdirectory(logging)

add_subdirectory(playground)
