add_library(gui STATIC
    ./src/winapi/windowproc.cpp
    ./src/winapi/windowproc.hpp)
target_include_directories(gui PUBLIC ./include)
target_link_libraries(gui PUBLIC ecs)
target_link_libraries(gui PUBLIC logging)

set(ISTOK_GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)
