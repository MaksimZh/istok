add_library(gui STATIC)
target_include_directories(gui PUBLIC ./include)
target_link_libraries(gui PUBLIC ecs)
target_link_libraries(gui PUBLIC logging)

target_sources(gui PUBLIC
    ./include/istok/gui/callback_types.hpp
)

set(ISTOK_GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)

if (WIN32)
    add_subdirectory(src/winapi)
endif (WIN32)
