find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_library(gui INTERFACE)
target_include_directories(gui INTERFACE ./)
target_link_libraries(gui INTERFACE logging tools ecs dwmapi opengl32 GLEW::GLEW)
