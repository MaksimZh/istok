find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_library(gui STATIC
    ./gui/winapi/window.cpp
)
target_include_directories(gui PUBLIC ./)
target_link_libraries(gui PUBLIC logging tools ecs dwmapi opengl32 GLEW::GLEW)
