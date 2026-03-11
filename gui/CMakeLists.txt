add_library(gui STATIC)
target_include_directories(gui PUBLIC ./include)
target_include_directories(gui PRIVATE ./src)
target_link_libraries(gui PUBLIC ecs)
target_link_libraries(gui PUBLIC logging)

target_sources(gui PRIVATE
    ./include/istok/gui.hpp
    ./include/istok/gui/base.hpp
    ./src/gui.cpp
)

set(ISTOK_GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)

if (WIN32)
    add_subdirectory(src/winapi)
endif (WIN32)
