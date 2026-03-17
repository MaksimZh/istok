set(ISTOK_GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
find_package(Catch2 REQUIRED)
find_package(Trompeloeil REQUIRED)

add_library(gui STATIC)
target_include_directories(gui PUBLIC ./include)
target_include_directories(gui PRIVATE ./src)
target_link_libraries(gui PUBLIC ecs)
target_link_libraries(gui PUBLIC logging)

target_sources(gui PRIVATE
    ./include/istok/gui/base.hpp
    ./include/istok/gui.hpp
    ./src/gui.cpp
)

if (WIN32)
    add_subdirectory(src/winapi)
endif (WIN32)
