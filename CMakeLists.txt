cmake_minimum_required(VERSION 3.23)
if(WIN32)
    if(DEFINED ENV{VCPKG_ROOT} AND NOT CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "Path to vcpkg toolchain file")
    endif()
    if(NOT VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET x64-windows CACHE STRING "vcpkg target triplet")
    endif()
endif()

project(Istok)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(ecs)
add_subdirectory(gui)
add_subdirectory(logging)

add_subdirectory(playground)
